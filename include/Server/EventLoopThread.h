#pragma once

#include<functional>
#include<mutex>
#include<condition_variable>
#include<string>
#include<memory>
#include"EventLoop.h"
#include"Thread.h"
#include"noncopyable.h"
#include"Logger.h"


class EventLoopThread: noncopyable{
    /*
    这个类 等价一个线程  创建这个类就等于创建了一个线程 开启了一个事件循环
    
    */
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;

    //线程函数
    EventLoopThread(const ThreadInitCallBack &cb = ThreadInitCallBack(),
                    const std::string &name = std::string());
                    
    ~EventLoopThread();

    EventLoop* startLoop();


private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex  mutex_;
    std::condition_variable cond_;  

    ThreadInitCallBack callback_;
};