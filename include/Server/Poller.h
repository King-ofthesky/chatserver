#pragma once
#include<map>
#include<vector>
#include<unordered_map>
#include<iterator>

#include"Channel.h"
#include"noncopyable.h"
//抽象类 抽象poll和epoll
//多路事件分发器  io复用模块

class Poller:noncopyable{
    public:
    using ChannelList = std::vector<Channel*>;
    ChannelList channels_;

    Poller(EventLoop *loop);

    
    virtual ~Poller();
    //使用虚析构函数


    /**
    给io复用保留统一的接口  epoll_wait
    从epoll_wait中得到激活的事件  channel 保存events 和所属loop指针

    */
    virtual Timestamp poll(int timeOutMs,ChannelList *activeChannelList) = 0; 

    virtual void updateChannel(Channel *channel) = 0; //epoll_ctl 更新

    virtual void removeChannel(Channel *channel) = 0;//epoll_ctl 删除events
    //判断channel 是否在当前 Poller中
    bool hasChannel(Channel *channel) const;

    //获取默认poll  也就是epoll  
    static Poller* newDefaultPoller(EventLoop *loop);


    protected:
    using   ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channelmap_;

    private:

    EventLoop *ownerloop_;
};