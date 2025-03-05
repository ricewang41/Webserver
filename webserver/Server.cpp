#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "Util.h"
#include "base/Logging.h"

Server::Server(EventLoop* loop, int threadNum, int port)
  :  loop_(loop),
     threadNum_(threadNum),
     eventLoopThreadPoll_(new EventLoopThreadPool(loop_, threadNum)),
     started_(false),
     acceptChannel_(new Channel(loop_)),
     port_(port),
     listenFd_(socket_bind_listen(port_)){
        acceptChannel_->setfd(listenFd_);
        handle_for_sigpipe();//防止在写入关闭的套接字时程序崩溃
        //监听套接字设置为非阻塞模式
        if(setSocketNonBlocking(listenFd_) < 0){
            perror("set socket non block failed");
            abort();
        }
     }
    
void Server::start(){
    eventLoopThreadPoll_->start();
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
    acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
    loop_->addToPoller(acceptChannel_, 0);//开始监控事件
    started_ = true;
}

void Server::handNewConn(){
    //初始化一个 sockaddr_in 结构来存储客户端的地址信息，并设置长度。
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr,&client_addr_len))>0){
        EventLoop* loop = eventLoopThreadPoll_->getNextLoop();
        LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
        //限制服务器最大并法连接数
        if(accept_fd >= MAXFDS){
        close(accept_fd);
        continue;
        }
        //设为非阻塞方式
        if(setSocketNonBlocking(accept_fd) < 0){
            LOG << "Set non block failed!";
            // perror("Set non block failed!");
            return;
        }

        setSocketNodelay(accept_fd);
        //将 HttpData 对象与 Channel 关联，并将其加入到事件循环中
        shared_ptr<HttpData> req_info(new HttpData(loop_, accept_fd));
        req_info->getChannel()->setHolder(req_info);
        loop->queueINLoop(std::bind(&HttpData::newEvent, req_info));
    }
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}