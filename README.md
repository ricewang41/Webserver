本项目是一个高性能的WEB服务器，使用C++实现，项目底层采用了muduo库核心的设计思想，多线程多Reactor的网络模型。代码主体的编写是通过One Loop per Thread思想来编写的，借鉴了很多muduo库里的设计。
##开发环境
* linux kernel version5.15.0-113-generic (ubuntu 22.04.6)
* gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
* cmake version 3.22
##目录结构：
webserver/
├── base/ # 基础功能源文件
├── test/ # 测试用例
│ ... # 主体功能源文件 
├── CMakeLists.txt # CMake 构建文件
└── README.md # 项目说明文件
##功能模块划分：
- 事件轮询与分发模块：`EventLoop.*`、`Channel.*`、`Poller.*`、`EPollPoller.*`负责事件轮询检测，并实现事件分发处理。`EventLoop`对`Poller`进行轮询，`Poller`底层由`EPollPoller`实现。
- 线程与事件绑定模块：`Thread.*`、`EventLoopThread.*`、`EventLoopThreadPool.*`绑定线程与事件循环，完成`one loop per thread`模型。
- 日志模块负责记录服务器运行过程中的重要信息，帮助开发者进行调试和性能分析。
##编译方式：
git clone https://github.com/ricewang41/webserver.git
   cd webserver

 mkdir build &&
   cd build &&
   cmake .. &&
   make -j8
