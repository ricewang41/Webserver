#pragma once
//不可拷贝类 禁止外界在子类没有定义的情况下进行赋值和拷贝构造操作
class noncopyable{
    protected:
    noncopyable() {};
    ~noncopyable() {};

    private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};