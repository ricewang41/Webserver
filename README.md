# webserver（Linux系统下的轻量级服务器）
## 项目介绍：
本项目是一个高性能的WEB服务器，使用C++实现，项目底层采用了muduo库核心的设计思想，多线程多Reactor的网络模型。代码主体的编写是通过One Loop per Thread思想来编写的，借鉴了很多muduo库里的设计。
## 开发环境：
* ubuntu 20
* cmake version 3.22
## 目录结构：
```shell
webserver/
├── base/ # 基础功能源文件
├── test/ # 测试用例
│ ... # 主体功能源文件 
CMakeLists.txt # CMake 构建文件
README.md # 项目说明文件
```
## 功能模块划分：
- 事件轮询与分发模块：`EventLoop.*`、`Channel.*`、`Poller.*`、`EPollPoller.*`负责事件轮询检测，并实现事件分发处理。`EventLoop`对`Poller`进行轮询，`Poller`底层由`EPollPoller`实现。
- 线程与事件绑定模块：`Thread.*`、`EventLoopThread.*`、`EventLoopThreadPool.*`绑定线程与事件循环，完成`one loop per thread`模型。
- 日志模块负责记录服务器运行过程中的重要信息，帮助开发者进行调试和性能分析。
## 做的优化工作：
- 基于小根堆的高效定时器。
- 双缓冲区的日志写入。
- RAII思想的体现：智能指针和锁的应用。
## 编译方式：
```bash
git clone https://github.com/ricewang41/webserver.git
cd webserver
```

```bash
mkdir build &&
cd build &&
cmake .. &&
make -j8
```
## 开发中遇到的困难：
1.一开始对于技术的理解并不深刻，在双日志缓冲区、线程池的编写过程中遇到了问题，这部分我主要是从论坛和github上找技术文档，多看多总结。
2.在功能的实现上，尤其是连接的建立这个模块，由于工程能力不足，会出现各种bug。首先我会确保总体逻辑的正确性，然后通过日志系统来定位出错的地方，最终实现debug的目标。
