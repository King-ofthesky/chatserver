#pragma once 

#include<vector>
#include<memory>
#include"Logger.h"
#include"noncopyable.h"
#include"EventLoop.h"
#include"EventLoopThread.h"


/*
    用于管理thread

*/
class EventLoopThreadPoll:noncopyable{
public:
using ThreadInitCallBack = std::function<void(EventLoop*)>;

/*
创建 线程池   事件循环的线程池  用于管理eventloop所在的thread，创建线程池时需要创建一个baseloop  
baseloop 是用于执行Accept 的事件循环的  用于执行accept函数  并且把得到的连接 都分发到

线程池 内部是一个 Vector<std::unique_ptr<EventLoopThread>   一个EventLoopThread的智能指针的数组
        和一个 std::vector<EventLoop*>     一个指向EventLoop 的指针  
        其中EventLoopThread  和 EventLoop  一一对应



*/
EventLoopThreadPoll(EventLoop *baseloop,const std::string &nameArg);
~EventLoopThreadPoll();

void setThreadNum(int numThreads){numThreads_ = numThreads;};

void start(const ThreadInitCallBack &cb= ThreadInitCallBack());

//在多线程环境中 mainloop 以轮询的方式分配channel给subloop
EventLoop* getNextLoop();
 
std::vector<EventLoop*> getAllLoops();

bool started()const{return started_;};
const std::string name()const{return name_;};


private:
EventLoop *baseLoop_;//
std::string name_;
bool started_;
int numThreads_;
int next_;//轮询
std::vector<std::unique_ptr<EventLoopThread>> threads_;//所有创建的线程
std::vector<EventLoop*> eventloops_;//所有eventloop的指针


 

};