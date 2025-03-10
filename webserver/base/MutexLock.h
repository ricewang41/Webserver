#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock : noncopyable{
    public:
    MutexLock() {pthread_mutex_init(&mutex , NULL);}//初始化互斥锁
    ~MutexLock(){
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }
    void lock() {pthread_mutex_lock(&mutex);}
    void unlock() {pthread_mutex_unlock(&mutex);}
    pthread_mutex_t *get() {return &mutex;}

    //创建互斥锁
    private:
    pthread_mutex_t mutex;
    friend class Condition;
};

class MutexLockGuard : noncopyable{
    public:
    explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
    ~MutexLockGuard() { mutex.unlock(); }

    private:
    MutexLock &mutex;
};