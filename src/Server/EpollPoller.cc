#include"EpollPoller.h"
#include<errno.h>
#include"Channel.h"

//用于管理 
const int knew = -1;  //channel 还没有加入到poll  epoll_ctl 
const int Kadded = 1;  //channel已经在poll中        epoll_ctl
const int kdeleted = 2; //channel已经从epoll中删除  epoll_ctl  

EpollPoller::EpollPoller(EventLoop *loop):
    Poller(loop),//通过传入的loop指针初始化基类
    epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),//创建epoll 
    //EPOLL_CLOEXEC参数  当父进程fock时  子进程继承fd  父进程自动close  fd  也就是把fd传递给子进程了
    eventlist_(kInitEventListSize)
{
    if(epoll_fd_ <0){  //创建失败
    Logger::LOG_ERROR("EpollPoller.cc   EpollPoller   epoll_fd_ < 0  epoll创建失败");
        exit(-1);
    }   
}

EpollPoller::~EpollPoller(){
    ::close(epoll_fd_);
}


//基类抽象方法
//没问题
Timestamp EpollPoller::poll(int timeOutMs,ChannelList *activeChannelList) { // epoll_wait
    // LOG_ERROR

    // fd  数组起始地址  数组长度  timeout
    // std::string info = "EpollPoller.cc poll  执行epoll_wait epoll_fd_ 值为"+ std::to_string(epoll_fd_);

    // Logger::LOG_INFO(info.c_str());
    // int numEvents = ::epoll_wait(epoll_fd_,&(*eventlist_.begin()),static_cast<int>(eventlist_.size()),timeOutMs); 
    int numEvents = ::epoll_wait(epoll_fd_, eventlist_.data(), static_cast<int>(eventlist_.size()), timeOutMs);

    // std::string  info2 = "EpollPoller.cc poll  执行epoll_wait的值numEvents为---- "+ std::to_string(numEvents);
    // Logger::LOG_INFO(info2.c_str());
    /*
    static_cast<int>(eventlist_.size())
    eventlist_.size()：eventlist_ 是一个 std::vector<> 类型的容器，size() 是该容器提供的成员函数，用于返回容器中元素的数量。这行代码的作用
    是获取 eventlist_ 容器中的元素数量。static_cast<int>(eventlist_.size())：static_cast<int> 是 C++ 中的类型转换操作符，它将括号内的表达式
    转换为指定的类型。在此代码中，它将 eventlist_.size() 的结果转换为 int 类型。由于 size() 返回的是无符号整数类型 std::size_t，使用 
    static_cast<int> 可以将其转换为有符号整数 int，以便进行后续的操作。综合起来，这段代码的目的是获取 eventlist_ 容器中元素的数量，并将其转换为
    有符号整数类型 int。这样可以方便地在后续的代码中使用该数量进行各种计算和逻辑判断。
    */
    int saveerr = errno;
    if(numEvents>0){
        // LOG_INFO
        // Logger::LOG_INFO("EpollPoller.cc  epoll_wait   numEvents ==  %d",numEvents);
        fillActiveChannels(numEvents,activeChannelList);
        if(numEvents == eventlist_.size()){
            eventlist_.resize(numEvents * 2);
        }

    }else if(numEvents == 0){//没事件发生
        //LOG_DEBUG

    }else{
        //判断错误情况
        if(saveerr != EINTR){ // 外部中断
            errno = saveerr;

        }
    }
    return Timestamp::now();
}
//对应eoll_ctl
void EpollPoller::updateChannel(Channel *channel) {   //使用了EpollPoller::update  也就是epoll_ctl
    Logger::LOG_INFO("EpollPoller.cc updateChannel   do");
    //更新channel 的信息
    const int index = channel->index();
    if(index == knew || index == kdeleted){  //刚创建的channel 或者已经删除的channel

        int fd = channel->fd();
        if(index == knew){ // 刚创建的 channel
        Logger::LOG_INFO("EpollPoller.cc updateChannel  vector 中添加fd do");

        channelmap_[fd] = channel; //向map 中添加fd

        Logger::LOG_INFO("EpollPoller.cc updateChannel  vector 中添加fd ok");
        }else{  

        }
        channel->setIndex(Kadded);  //刚注册的事件  添加到epoll
        Logger::LOG_INFO("EpollPoller.cc updateChannel update(EPOLL_CTL_ADD,channel)  do");
        update(EPOLL_CTL_ADD,channel);//channel执行update 也就是epoll_ctl add
        Logger::LOG_INFO("EpollPoller.cc updateChannel update(EPOLL_CTL_ADD,channel)  ok");

    }else{  //说明已经注册过channel了
        int fd = channel->fd();
        (void)fd;//避免编辑器警告
        if(channel->isNoneEvent()){  //在更新时发现对任何事件不感兴趣  也就是删除  
            update(EPOLL_CTL_DEL,channel);   //从epoll中删除
            channel->setIndex(kdeleted);
        Logger::LOG_INFO("EpollPoller.cc updateChannel update(EPOLL_CTL_DEL,channel)  ok");
        }else{
            update(EPOLL_CTL_MOD,channel);//在epoll中修改对应的events   epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
            Logger::LOG_INFO("EpollPoller.cc updateChannel update(EPOLL_CTL_MOD,channel)  ok");
        }
    }
     Logger::LOG_INFO("EpollPoller.cc updateChannel   ok");
}

void EpollPoller::removeChannel(Channel *channel) {  //把channel 从epoll中删除
    int fd = channel->fd();
    int index = channel->index();

    //根据fd获取到channel_的迭代器  
    auto it = std::find_if(channels_.begin(), channels_.end(), 
        [fd](const Channel* ch) {   //lambda 表达式 判断fd是否是属于当前channel
         return ch->fd() == fd;
        }   
    );

    //删除
    if(it != channels_.end())
    {
        channels_.erase(it);//
    }

    if(index == Kadded){
        update(EPOLL_CTL_DEL,channel); // 从epoll中删除
    
    }

    channel->setIndex(knew);
}




//填写活跃连接
void EpollPoller::fillActiveChannels(int numevents,ChannelList* activechannellist)const{
    for(int i = 0;i < numevents;++i){
        Channel *channel = static_cast<Channel*>(eventlist_[i].data.ptr);//从epoll_event中得到 
        channel->set_revents(eventlist_[i].events);
        activechannellist->push_back(channel);
    }
}
//更新channel通道
void EpollPoller::update(int operation,Channel* channel){
    //通过channel指针 和
    struct epoll_event  event;//epoll_event
    memset(&event,0,sizeof event);  //epoll_event对象初始化0
    event.events = channel->events(); //设置events
    event.data.ptr = channel;
    int fd = channel->fd();
    if(::epoll_ctl(epoll_fd_,operation,fd,&event)<0){
        //
        if(operation == EPOLL_CTL_DEL){
            // LOG_ERROR
        }else{
            // LOG_ERROR
        }
    }
}

