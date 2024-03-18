#include"Poller.h"

Poller::Poller(EventLoop *loop)
    :ownerloop_(loop)
{
    //创建poll需要传入一个loop指针来告诉poller自己是属于哪个线程的 可以通过loop指针在当前线程执行回调函数

}

Poller::~Poller( )
{

}


bool Poller::hasChannel(Channel *channel) const
{  
    /**
根据fd  判断有没有channel  
有意味着在当前线程
没有意味着不在当前线程

*/
//    using   ChannelMap = std::unordered_map<int,Channel*>;

    /**
    不能使用map[fd] 这样查找  在吗map和unorder_map中 如果没找到 会默认构造一个值插入进去
    */
    auto it= channelmap_.find(channel->fd());
    //std::map<int,Channel*>::iterator it;
    return it !=channelmap_.end() && it->second == channel;
}


