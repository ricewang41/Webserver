#pragma once
#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"
//CountDownLatch 允许一个或多个线程等待一组操作在其他线程中完成。
class CountDownLatch : noncopyable {
    public:
    explicit CountDownLatch(int count);
    ~CountDownLatch() {};
    void wait();
    void countDown();

    private:
    //线程同步的互斥锁，mutable允许常量成员函数修改这些变量的数值
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};