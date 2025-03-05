#pragma once
#include <errno.h>
#include <pthread.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>
#include "MutexLock.h"
#include "noncopyable.h"
//线程同步和协调类
class Condition : noncopyable {
    public:
    explicit Condition(MutexLock &_mutex) : mutex(_mutex) {
        pthread_cond_init(&cond, NULL);
    }
    ~Condition() { pthread_cond_destroy(&cond); }
    //挂起并释放互斥锁
    void wait() { pthread_cond_wait(&cond, mutex.get()); }
    //唤醒（所有）等待线程
    void notify() { pthread_cond_signal(&cond); }
    void notifyALL() { pthread_cond_broadcast(&cond); }
    //让线程最多等待seconds秒 到时间返回true
    bool waitForSecond(int seconds){
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

    private:
    MutexLock &mutex;
    pthread_cond_t cond;
};