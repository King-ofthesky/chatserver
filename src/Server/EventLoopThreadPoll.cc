#include"EventLoopThreadPoll.h"

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop *baseloop,const std::string &nameArg)
                    :baseLoop_(baseloop)
                    ,name_(nameArg)
                    ,started_(false)
                    ,numThreads_(0)
                    ,next_(0)

{

}

EventLoopThreadPoll::~EventLoopThreadPoll(){

}


void EventLoopThreadPoll::start(const ThreadInitCallBack &cb){
    
    started_ = true;
    /*

    */
    for(int i=0;i<numThreads_;++i){

        char buf[name_.size()+32];   //线程名
        snprintf(buf,sizeof (buf), "%s%d",name_.c_str(),i);//buf=  name_+"i"
        EventLoopThread *t = new EventLoopThread(cb,buf);//创建一共EventLoopThread线程

        threads_.push_back(std::unique_ptr<EventLoopThread>(t));//threads_ 保存每个EventLoopThread的指针 
        
        eventloops_.push_back(t->startLoop());//通过线程调用 startloop创建一个threadloop  然后eventLoops保存这个threadloop的指针
    }
    //如果=0 服务端只有一个程序运行  
    if(numThreads_==0 && cb){
            cb(baseLoop_);//执行回调函数
    }
}

//在多线程环境中 mainloop 以轮询的方式分配channel给subloop
EventLoop* EventLoopThreadPoll::getNextLoop(){
    EventLoop* loop = baseLoop_;
    if(!eventloops_.empty()){
        loop = eventloops_[next_];//
        ++next_;
        if(next_ >= eventloops_.size()){
            next_ = 0;
        }
        
    }
    return loop;
}


std::vector<EventLoop*> EventLoopThreadPoll::getAllLoops(){
    if(eventloops_.empty()){//空你要是没有sub线程 只有一个mianthread
        return std::vector<EventLoop*>(1,baseLoop_);//大小为1 保存了baseloop_
    }else{
        return eventloops_;
    }
}