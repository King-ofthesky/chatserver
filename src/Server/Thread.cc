#include<semaphore.h>

#include"Thread.h"
#include"CurrentThread.h"
#include<sstream>
std::atomic_int Thread::numCreated_ = 0;//用于记录线程数量 

Thread::Thread(ThreadFunc func,const std::string &name)
    :started_(false)
    ,joined_(false)
    ,tid_(0)
    ,func_(std::move(func))
    ,name_(name)

{
    setDefaultName();
}
Thread::~Thread(){
    if(started_ &&!joined_){
        thread_->detach();// 
    }
}
    
void Thread::start(){
    Logger::LOG_INFO("Thread.cc start  开始创建线程" );
    started_ = true;//表示线程已经开始。
    sem_t  sem ;//信号量
    sem_init(&sem,false,0);//信号量初始化0    sem = 0
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){ // 创建线程对象
        Logger::LOG_INFO("Thread.cc start  获取线程id  ");
        tid_ = currentthread::tid();//在创建线程对象后 获取当前线程id
        std::stringstream ss ;
        ss << tid_;
        sem_post(&sem);//信号量增加1   sem  0 ->1  
        Logger::LOG_INFO("Thread.cc start  开始执行线程函数  ");
        func_(); //执行线程函数  也就是  thread()  此时线程在开始运行
    }));

    //
    sem_wait(&sem);//主线程阻塞在sem   等待上面lambda 函数执行完毕  
}

//封装 thread的join
void Thread::join(){  
    joined_ = true;
    thread_->join();
    
}

//设置线程名字 debug好用
void Thread::setDefaultName(){
    int number = ++numCreated_;
    if(name_.empty()){
        char buf[32] = {0};
        snprintf(buf,sizeof buf,"Thread%d",number);
        name_ = buf;
    }

}  