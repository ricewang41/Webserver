#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "base/Logging.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int createEventFd() {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
  :  looping_(false),
     poller_(new Epoll()),
     wakeupFd_(createEventFd()),
     quit_(false),
     eventHandling_(false),
     callingPendingFunctors_(false),
     threadId_(CurrentThread::tid()),
     pwakeupChannel_(new Channel(this, wakeupFd_)){
        if(t_loopInThisThread){
            // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
            // thread " << threadId_;
        }else{
            t_loopInThisThread = this;
        }
    //对读事件感兴趣并且采取边缘触发，将读事件绑定到pwakeupchannel上
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
    pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
    poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn(){
    updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop(){
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
      LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);

}

void EventLoop::runInLoop(Functor&& cb){
    if(isInLoopThread())
      cb();
    else
      queueINLoop(std::move(cb));
}

void EventLoop::queueINLoop(Functor&& cb){
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if(!isInLoopThread() || callingPendingFunctors_) wakeup();
}
//事件循环
void EventLoop::loop(){
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;
    std::vector<SP_Channel> ret;
    while(!quit_){
        ret.clear();
        ret = poller_->poll();
        eventHandling_ = true;
        for(auto& it : ret){
            it->handleEvents();
        }
        eventHandling_ = false;
        doPendingFunctors();
        poller_->handleExpired();
    }
    looping_ = false;
}
//执行所有等待处理的函数
void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(size_t i = 0; i < functors.size(); i++){
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::quit(){
    quit_ = true;
    if(!isInLoopThread()){
        wakeup();
    }
}
