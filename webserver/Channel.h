#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"
//用于封装与文件描述符（如套接字）相关的事件处理逻辑,主要负责监听事件和处理读写事件
class EventLoop;
class HTtpData;
class Channel {
    private:
    typedef std::function<void()> CallBack;
    EventLoop* loop_;
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;//实际发生的事件类型
    __uint32_t lastEvents_;
    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;

    private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();

    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

    public:
    Channel(EventLoop* loop);
    Channel(EventLoop* loop, int fd);
    ~Channel();
    int getfd();
    void setfd(int fd);

    void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
    std::shared_ptr<HttpData> getHolder() {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
        }
    //读事件的回调函数
    void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
    void setWriteHandler(CallBack &&writeHandler) {
      writeHandler_ = writeHandler;
    }
    void setErrorHandler(CallBack &&errorHandler) {
      errorHandler_ = errorHandler;
    }
    void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }
    
    void handleEvents() {
        events_ = 0;
        if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
            events_ = 0;
            return;
        }
        if((revents_ & EPOLLERR)){
            if (errorHandler_) errorHandler_();
            events_ = 0;
            return;
        }
        if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
            handleRead();
        }
        if(revents_ & EPOLLOUT){
            handleWrite();
        }
        handleConn();
    }
    void handleRead();
    void handleWrite();
    void handleError(int fd, int err_num, std::string short_msg);
    void handleConn();

    void setRevent(__uint32_t ev) { revents_ = ev; }

    void setEvents(__uint32_t ev) {events_ = ev; }
    __uint32_t &getEvents() {return events_; }

    bool EqualAndUpdateLastEvents() {
        bool ret = (lastEvents_ == events_ );
        lastEvents_ = events_;
        return ret;
    }

    __uint32_t getLastEvents() { return lastEvents_; }

};
typedef std::shared_ptr<Channel> SP_Channel;