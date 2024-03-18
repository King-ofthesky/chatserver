#pragma once

#include<functional>
#include<thread>
#include<memory>
#include<string>
#include<atomic>
#include"Logger.h"
#include"noncopyable.h"
/*
线程类

线程类包含  线程函数   


*/

class Thread : noncopyable{
public:
    using ThreadFunc = std::function<void()>;  //线程函数  需要多参数 用bind

    explicit Thread(ThreadFunc func,const std::string &name = std::string());
    ~Thread();
    
    void start();
    void join();

    bool started()const{return started_;};
    pid_t tid()const{return tid_;};
    const std::string name()const{return name_;};

    static int numCreated(){return numCreated_;};

private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;  
    static std::atomic_int  numCreated_;//用于记录线程数量 

    void setDefaultName();   
};