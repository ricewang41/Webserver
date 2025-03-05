#pragma once
#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "base/Logging.h"
#include "base/noncopyable.h"

class EventLoopThreadPool : noncopyable{
    public:
    EventLoopThreadPool(EventLoop* baseloop, int numthreads);

    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPoll"; }
    void start();

    EventLoop* getNextLoop();

    private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;//线程池中每个线程对应的eventloop指针
};