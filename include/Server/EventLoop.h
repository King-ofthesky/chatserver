#pragma once 

class Channel;
class Poller;

#include<atomic>
#include<vector>
#include<memory>
#include<mutex>
#include<functional>

#include"Timestamp.h"
#include"CurrentThread.h"
#include"noncopyable.h"
//事件循环类  包含 Channel  和Pooler(epool的抽象)     
/*
    每个线程都持有一个Eventloop  
    Eventloop对象会有一个当前线程的id 用来判断 自己属于哪个线程
    有一个 std::vector<Functor> pendingFunctors_;  是一个保存了所有需要执行的回调函数的队列
    有一个 std::vector<Channel*>   activeChannels_ 保存了当前loop所属的channel的队列





*/


class EventLoop:noncopyable{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollReturnTime()const{return pollReturnTime_;};
    //在当前loop中执行
    void runInLoop(Functor callback);
    //把callback放入队列中 唤醒所在的loop执行
    void queueInLoop(Functor callback);
    //唤醒loop所在的线程
    void wakeup();
    //调用poller的方法 
    //更新channel
    //Eevntloop 传入一个channel  让poller -> epoll 执行自己的update方法
    void updateChannel(Channel* channel);
    //Eevntloop 传入一个channel  让poller -> epoll 执行自己的remove方法
    void removeChannel(Channel* channel);
    //Eevntloop 传入一个channel  让poller -> epoll 执行自己hasChannel
    bool hasChannel(Channel* channel);

    //判断eventloop对象是否在自己的线程里  因为是多个线程
    bool isInLoopThread()const{
        return threadId_ == currentthread::tid();
    }

private:
    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_; //原子操作布尔值
    std::atomic_bool quit_; //判断是否退出事件循环  //所有loop共用这个变量
    std::atomic_bool eventHandling_;

    const pid_t threadId_;//记录loop所在线程的id
    Timestamp pollReturnTime_;//poll返回发生事件的时间

    std::unique_ptr<Poller>  poller_;
    int wakeupFd_;//当mainloop获取channel后 通过轮询算法分发给subloop
    std::unique_ptr<Channel> wakeupChannel_;


    ChannelList activeChannels_;//保存当前loop所属的channel

    Channel *currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_;//标识是否需要执行回调操作
    std::vector<Functor> pendingFunctors_;//存储loop需要执行的所有的回调操作
    std::mutex  mutex_;//互斥锁 保证pendingFunctors_安全



    void handleRead();
    void doPendingFunctors();//执行回调


};