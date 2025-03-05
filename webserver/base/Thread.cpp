// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "CurrentThread.h"

#include<iostream>
using namespace std;

namespace CurrentThread{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "default";
}
//调用系统调用获得tid信息
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }
//缓存当前线程的 ID 和其字符串表示。
//如果 t_cachedTid 为 0，则调用 gettid 获取线程 ID，并使用 snprintf 将其格式化为字符串。
void CurrentThread::cacheTid() {
    if(t_cachedTid == 0){
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}

//在线程中保留name，tid这些数据 func_(线程需要执行的函数) latch_(同步计数器的🔓)

struct ThreadData {
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func, const string& name, pid_t* tid,
               CountDownLatch* latch)
        :func_(func), name_(name), tid_(tid), latch_(latch) {}
    
    //runInThread()函数设置线程名称和tid后，通知主线程并且执行func
    void runInThread() {
        *tid_ = CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* obj){
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}

Thread::Thread(const ThreadFunc& func, const string& n) 
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(func),
      name_(n),
      latch_(1){
        setDefaultName();
      }

Thread::~Thread(){
    if(started_ && !joined_)
    pthread_detach(pthreadId_);
}

void Thread::setDefaultName() {
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread");
    name_ = buf;
  }
}
//创建线程并等待线程启动，如果线程创建成功，pthread_create会返回0，否则是一串错误代码
void Thread::start() {
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthreadId_, NULL, &startThread, data)){
        started_ = false;
        delete data;
    }else {
        latch_.wait();
        assert(tid_ > 0);
    }
}
//等待线程结束返回pthread——join的结果
int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

