#include "AsyncLogging.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include "LogFile.h"

AysncLogging::AysncLogging(std::string logFileName_, int flushInterval) 
    : flushINterval_(flushInterval),
      running_(false),
      basename_(logFileName_),
      thread_(std::bind(&AysncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_(),
      latch_(1){
        assert(logFileName_.size() > 1);
        currentBuffer_->bzero();
        nextBuffer_->bzero();
        buffers_.reserve(16);
}
//写入当前缓冲区，如果当前缓冲区空间不足，则将当前缓冲区重置或者切换到下一个缓冲区，通过move操作避免了不必要的复制
void AysncLogging::append(const char* logline, int len) {
    MutexLockGuard lock(mutex_);
    if(currentBuffer_->avail() > len)
    currentBuffer_->append(logline, len);
    else{
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();
        if(nextBuffer_)
        currentBuffer_ = std::move(nextBuffer_);
        else
        currentBuffer_.reset(new Buffer);
        currentBuffer_->append(logline, len);
        cond_.notify();
    }

}

void AysncLogging::threadFunc() {
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector bufferToWrite;
    bufferToWrite.reserve(16);
    while(running_){
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(bufferToWrite.empty());
        {
            MutexLockGuard lock(mutex_);
            if(buffers_.empty())
            cond_.waitForSecond(flushINterval_);
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();

            currentBuffer_ = std::move(newBuffer1);
            bufferToWrite.swap(buffers_);
            if(!nextBuffer_){
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!bufferToWrite.empty());

        if(bufferToWrite.size() > 25){
            bufferToWrite.erase(bufferToWrite.begin()+2, bufferToWrite.end());
        }

        for(size_t i = 0; i<bufferToWrite.size(); ++i){
            output.append(bufferToWrite[i]->data(), bufferToWrite[i]->length());
        }

        if(bufferToWrite.size() > 2){
            bufferToWrite.resize(2);
        }

        if(!newBuffer1){
            assert(!bufferToWrite.empty());
            newBuffer1 = bufferToWrite.back();
            bufferToWrite.pop_back();
            newBuffer1->reset();
        }


        if(!newBuffer2){
            assert(!bufferToWrite.empty());
            newBuffer2 = bufferToWrite.back();
            bufferToWrite.pop_back();
            newBuffer2->reset();
        }

        bufferToWrite.clear();
        output.flush();
    }
    output.flush();
}