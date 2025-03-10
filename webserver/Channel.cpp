#include "Channel.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include <queue>

#include "Epoll.h"
#include "EventLoop.h"
#include "Util.h"

using namespace std;

Channel::Channel(EventLoop* loop)
  :  loop_(loop), fd_(0) ,events_(0), lastEvents_(0){}

Channel::Channel(EventLoop* loop, int fd)
  :  loop_(loop), fd_(fd), events_(0), lastEvents_(0) {}

Channel::~Channel() {

}

int Channel::getfd() { return fd_; }
void Channel::setfd(int fd) { fd_ = fd; }

void Channel::handleRead() {
    if(readHandler_){
        readHandler_();
    }
}

void Channel::handleWrite() {
  if (writeHandler_) {
    writeHandler_();
  }
}

void Channel::handleConn() {
  if (connHandler_) {
    connHandler_();
  }
}