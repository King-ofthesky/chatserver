#include"Poller.h"
#include"EpollPoller.h"
#include<stdlib.h>
#include"Logger.h"
Poller* Poller::newDefaultPoller(EventLoop *loop){
    /*
    ::getenv("MODUO_USE_POLL") 是一个 C++ 中使用标准库函数 getenv() 获取环境变量的操作。
    在这个特定的例子中，它是用来获取名为 "MODUO_USE_POLL" 的环境变量的值。getenv() 函数可以用于获取指定名称的环境变量的值。
    如果环境变量存在且设置了对应的值，那么 getenv() 函数将返回一个指向该值的指针。如果环境变量不存在或者没有设置值，则返回空指针（nullptr）。
    在代码中调用 ::getenv("MODUO_USE_POLL") 会尝试获取名为 "MODUO_USE_POLL" 的环境变量的值。你可以根据返回的指针来判断环境变量的状态并做相
    应的处理。*/
    //直接生成epoll实例
    //因为我只实现了epoll所以直接使用epoll了
    
    // if(::getenv("MODUO_USE_POLL"))
    // {
        // Logger::LOG_INFO("DefaultPollerer.h poller 生成失败");
        // return nullptr;//生成poll实例
    // }else
    // {
        Logger::LOG_INFO("DefaultPollerer.h poller 生成成功");
        return new EpollPoller(loop);//生成epoll实例
    // }
}