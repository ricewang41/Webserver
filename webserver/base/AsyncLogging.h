#pragma once
#include <functional>
#include <string>
#include <vector>
#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "Thread.h"
#include "noncopyable.h"

class AysncLogging : noncopyable {
    public:
    AysncLogging(const std::string basename, int flushInterval = 2);
    ~AysncLogging() {
        if(running_)
        stop();
    }
    void append(const char* logline, int len);
    //异步记录，停止时使用join方法等待记录线程结束,会阻塞主线程直到日志记录结束
    void start(){
        running_ = true;
        thread_.start();
        latch_.wait();
    }
    void stop(){
        running_ = false;
        cond_.notify();
        thread_.join();
    }

    private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;
    const int flushINterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    CountDownLatch latch_;
};