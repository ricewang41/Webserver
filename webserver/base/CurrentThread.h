#pragma once
#include <stdint.h>

namespace CurrentThread{
    //存储和当前线程有关的信息 分别为线程标识符，字符串表示，实际长度，指向线程名称的指针
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();
    //如果当前线程被缓存 返回tid，否则进行缓存（通常不会发生没被缓存的情况）
    inline int tid(){
        if(__builtin_expect(t_cachedTid == 0, 0)){
            cacheTid();
        }
        return t_cachedTid;
    }
    //获得线程的有关信息
    inline const char* tidString(){
        return t_tidString;
    }

    inline int tidStringLength(){
        return t_tidStringLength;
    }

    inline const char*name(){
        return t_threadName;
    }

}