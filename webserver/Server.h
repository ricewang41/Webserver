#pragma once
#include <memory>
#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

class Server{
    public:
    Server(EventLoop* loop, int threadNum, int port);
    ~Server() {}
    EventLoop* getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(acceptChannel_); }

    private:
    EventLoop* loop_;//处理服务器的主要事件循环
    int threadNum_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPoll_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;//处理新的客户端连接
    int port_;
    int listenFd_;//服务器监听的fd
    static const int MAXFDS = 100000;
};