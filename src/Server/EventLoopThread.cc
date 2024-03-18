#include"EventLoopThread.h"



EventLoopThread::EventLoopThread(const ThreadInitCallBack &cb ,const std::string &name)
                :loop_(nullptr)
                ,exiting_(false)
                ,thread_(std::bind(&EventLoopThread::threadFunc,this))
                ,mutex_()
                ,cond_()
                ,callback_(cb)
{

}



EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    /**/
    if(loop_ != nullptr){
        loop_->quit();
        thread_.join();
    }
}


EventLoop* EventLoopThread::startLoop(){
    /*
    本质上是调用thraed()方法 创建一个线程  线程函数为threadFunc  每次start都是启动一个新的线程
    */
    thread_.start();//start方法会创建一个新线程 执行下面的threadFunc函数
    //创建一个loop指针 
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        //从threadFunc中获取lopp
        while(loop_ == nullptr){//当上面start创建的线程函数执行完毕时loop不等于nullptr
            cond_.wait(lock);
        }
        loop = loop_;//在threadFunc中创建的loop变量
        // std::cout<<"EventLoopThread.cc startLoop  success"<<std::endl;
    }
    
    return loop;
}
void EventLoopThread::threadFunc(){

    EventLoop loop;  //一个独立的loop  对应上面的线程
    
    if(callback_)    //先执行 initcallback 
    {
        callback_(&loop);
    }

   {
    std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        // cond_.notify_all();//通知一个等待在mutex上的
        
        cond_.notify_one();
        // std::cout<<"EventLoopThread.cc threadFunc  notify_all"<<std::endl;
   }
   loop.loop();//在此处进行事件循环  如果执行下去则说明循环结束
   std::unique_lock<std::mutex> lock(mutex_);
   loop_=nullptr;
}