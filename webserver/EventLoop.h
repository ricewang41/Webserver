#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "base/Thread.h"

#include <iostream>
using namespace std;

class EventLoop {
    public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor&& cb);//安排在事件循环中进行
    void queueINLoop(Functor&& cb);//在等待处理队列排队
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}
    void assertInLoopThread() { assert(isInLoopThread()); }
    void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getfd()); }
    void removeFromPoller(shared_ptr<Channel> channel) {
        poller_->epoll_del(channel);
    }
    void updatePoller(shared_ptr<Channel>channel, int timeout = 0){
        poller_->epoll_mod(channel, timeout);
    }
    void addToPoller(shared_ptr<Channel>channel, int timeout = 0){
        poller_->epoll_add(channel, timeout);
    }

    private:
    bool looping_;
    shared_ptr<Epoll> poller_;
    int wakeupFd_;
    bool quit_;
    bool eventHandling_;
    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;//等待处理的函数对象
    bool callingPendingFunctors_;//false表示执行完了带处理的函数
    const pid_t threadId_;//存储事件循环运行的线程id
    shared_ptr<Channel> pwakeupChannel_;//用于唤醒的channel实例

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();


};