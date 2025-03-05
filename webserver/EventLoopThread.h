#pragma once
#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
//创建一个新线程来进行循环,继承自不可拷贝类，防止多个线程共享同一个循环
class EventLoopThread : noncopyable{
    public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

    private:
    void threadfunc();
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};