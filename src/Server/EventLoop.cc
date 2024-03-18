#include"EventLoop.h"
#include<sys/eventfd.h>
#include<unistd.h>
#include<fcntl.h>
#include<memory>
#include"Logger.h"
#include<errno.h>
#include"DefaultPollerer.h"
#include"Channel.h"
//防止线程创建多个Eventloop
__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMS = 10000; // 超时10s
//创建wakeupfd 用notify唤醒subreactor处理新来的channel
int createEvnetfd(){
    int evefd = ::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evefd <0){
        //LOG_FATAL();    
    }
    return  evefd;
}

/*
    创建Eventloop  
    在Eventloop 构造函数中 Eventloop 会通过系统调用获得 创建Eventloop线程的线程id 
    在创建Eventloop时  会构建一个Poller对象  并把当前Eventloop的指针给Poller对象  Poller是io多路复用的抽象  目前只有Epoll
    也就是说有一个Eventloop就有一个Poller
    

*/
EventLoop::EventLoop() 
    :looping_(false)
    ,quit_(false)
    ,callingPendingFunctors_(false)
    ,threadId_(currentthread::tid())
    ,poller_(Poller::newDefaultPoller(this))
    ,wakeupFd_(createEvnetfd())
    ,wakeupChannel_(new Channel(this,wakeupFd_))
    ,currentActiveChannel_(nullptr)
    
{
    //LOG_DEBUG("EVENTLOOP CREATE BY %p in this %d");

    //设置weakupfd事件类型  和发生事件后的回调操作
    if(t_loopInThisThread){
        //LOG_
    }else{
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead,this));
}

EventLoop::~EventLoop(){
    //对所有事件不感兴趣
    wakeupChannel_->disableAll();
    //移除所有channel
    wakeupChannel_->remove();

    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead(){
    uint64_t one=  1;
    ssize_t  n =  read(wakeupFd_,&one,sizeof one);
    if(n != sizeof one ){
        //LOG_ERROR()
    }
   
}
/*

tcpserver 持有一个loop 
*/
void EventLoop::loop(){
    looping_= true;
    quit_ = false;
    // LOG_INFO();
    //开启事件循环
    while(!quit_){
    activeChannels_.clear();//channel中events事件全清除  size = 0
    //主要是监听 clientfd  和wakeupfd。
    //wakeupfd 是mainloop和subloop直间的通信
    /**
    连接建立 接受到数据  但是没有发生数据
    */
    pollReturnTime_ = poller_->poll(kPollTimeMS,&activeChannels_);//设置超时
    //eventHandling_ = true;//用于控制断言 可以不写
    // if(activeChannels_.size()==1 ){
    // Logger::LOG_INFO("EventLoop.cc  activeChannels_  size==  %s ",std::to_string(activeChannels_.size()).c_str());
    // }
    for(Channel *channel :activeChannels_){   //vecrot<Channel *>
    //poller监听哪些channel 发生了事件 然后上报给Eventloop，通知channel处理事件
        channel->handleEvent(pollReturnTime_);

    }
    //eventHandling_ = false;
    doPendingFunctors();//执行当前Eventloop所需要的回调操作 回调函数在pendingFunctors_中
    /*
        mainloop 用于执行accept  , 每次有连接从accept会得到一个fd   ，然后用channel打包fd；此时需要把channel分发给subloop
        wakeup subloop后 执行下面的方法

    */
    activeChannels_.clear();//channel中events事件全清除  size = 0
    }
    Logger::LOG_INFO("%s,%s, EVENTLOOP LOOP has quit",__FILE__,__LINE__);

}

//退出事件循环 loop在自己的线程中调用quit
void EventLoop::quit(){
    quit_ = true;//把原子变量设为true
    if(!isInLoopThread()){   //如果其他线程中调用quit 则唤醒mian线程  然后退出mainloop
        wakeup();
    }
}
/*
   void runInLoop(Functor callback);
    //把callback放入队列中 唤醒所在的loop执行
    void queueInLoop(Functor callback);
*/

 //在当前loop中执行
void EventLoop::runInLoop(Functor callback){
    if(isInLoopThread()){  //在当前loop中执行
        callback();
    }else{  //不在当前loop中执行  需要唤醒loop所在线程执行 callback   因为每一个loop中管理channel  需要对应的channel去执行
        queueInLoop(callback);  //把他放入callback队列  让对应的loop去执行
    }
}

//把callback放入队列中 唤醒所在的loop执行
void EventLoop::queueInLoop(Functor callback){
    {
        std::unique_lock<std::mutex> lock_(mutex_);
        pendingFunctors_.emplace_back(callback);  //emplace_back直接构造    push_back(拷贝构造)
    }
    //唤醒 线程去执行回调
    if(!isInLoopThread()||callingPendingFunctors_){  //如果不在这个线程    或者true
        wakeup();//唤醒loop所在的线程
    }
    
}

/*
    //唤醒loop所在的线程
    void wakeup();
*/
void EventLoop::wakeup(){//向wakeup fd 写数据唤醒 
     uint64_t one = 1;
     ssize_t  n = write(wakeupFd_,&one,sizeof(one));
     if(n != sizeof(one)){
        //LOG_ERROR();
     }
}
/*
    调用poller的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
*/

void EventLoop::updateChannel(Channel* channel){
    if(channel==nullptr)
    {
        Logger::LOG_INFO("EventLoop.cc  updateChannel   channel==nullptr ");
    }

    if(poller_==nullptr)
    {
        Logger::LOG_INFO("EventLoop.cc  updateChannel   poller_==nullptr ");
    }
    Logger::LOG_INFO("EventLoop.cc  updateChannel   do  poller_->updateChannel  ");

    
    poller_->updateChannel(channel);

    // Logger::LOG_INFO("EventLoop.cc  updateChannel       updateChannel   ok ");

}
void EventLoop::removeChannel(Channel* channel){
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel){
        return poller_->hasChannel(channel);
}

//执行回调函数
void EventLoop::doPendingFunctors(){
    // Logger::LOG_INFO("EventLoop.cc182  doPendingFunctors");
    std::vector<Functor> functors;
    if(pendingFunctors_.size()>0)
    {
        std::string info = "EventLoop.cc182  pendingFunctors_<Functor>"+std::to_string(pendingFunctors_.size());
            Logger::LOG_INFO(info.c_str());
    }

    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex>  lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor &func :functors){
        Logger::LOG_INFO("EventLoop.  doPendingFunctors  执行func");
        func();
        Logger::LOG_INFO("EventLoop.  doPendingFunctors  执行func 执行完毕");
    }
    
    callingPendingFunctors_ = false;
}

