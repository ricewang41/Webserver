#include "Epoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include "Util.h"
#include "base/Logging.h"


#include <arpa/inet.h>
#include <iostream>
using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;
//初始化epollfd文件描述符和events_存储epoll——wait调用返回的事件表
Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
    assert(epollFd_ > 0);
}
Epoll::~Epoll() {}

//注册描述符 包括获取描述符，添加定时器，更改channel类的事件状态并添加到epoll数组中
void Epoll::epoll_add(SP_Channel request, int timeout){
    int fd = request->getfd();
    if(timeout > 0){
        add_timer(request, timeout);
        fd2http_[fd] = request->getHolder();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();

    request->EqualAndUpdateLastEvents();

    fd2chan_[fd] = request;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) > 0){
        perror("epoll_add error");
        fd2chan_[fd].reset();
    }
}
//修改描述符状态 包括更新定时器状态，检查事件类型是否变化，不变化直接结束，变化则对应修改
void Epoll::epoll_mod(SP_Channel request, int timeout){
    if(timeout > 0) add_timer(request, timeout);
    int fd = request->getfd();
    if(!request->EqualAndUpdateLastEvents()){
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->getEvents();
        if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        perror("epoll_mod error");
        fd2chan_[fd].reset();}
    }
}
//从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request) {
    int fd = request->getfd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
      perror("epoll_del error");
    }
    fd2chan_[fd].reset();
    fd2http_[fd].reset();
}
//返回活跃事件
std::vector<SP_Channel> Epoll::poll() {
    while(true){
        int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
        if(event_count < 0) perror("epoll wait error");
        std::vector<SP_Channel> req_data = getEventsRequest(event_count);
        if(req_data.size() > 0) return req_data;
    }
}

void Epoll::handleExpired() { timerManager_.handleExpiredEvent(); }

//分发处理函数 获得epollwait返回的事件列表，赋给channel类返回
std::vector<SP_Channel>Epoll::getEventsRequest(int events_num){
    std::vector<SP_Channel> req_data;
    for(int i = 0; i<events_num; i++){
    //获取有事件产生的描述符
    int fd = events_[i].data.fd;

    SP_Channel cur_req = fd2chan_[fd];
    if(cur_req){
        cur_req->setRevent(events_[i].events);
        cur_req->setEvents(0);
        //加入线程池之前将timer和request分离
        req_data.push_back(cur_req);
    }else{
        LOG << "SP cur_req is invalid";
    }
    }
    return req_data;
}

void Epoll::add_timer(SP_Channel request_data, int timeout) {
  shared_ptr<HttpData> t = request_data->getHolder();
  if (t)
    timerManager_.addTimer(t, timeout);
  else
    LOG << "timer add fail";
}