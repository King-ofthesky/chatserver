//日志系统
#pragma once
#include<string>
#include<iostream>
//#####
#include"noncopyable.h"
#include"Timestamp.h"
//定义日志级别
/*
一般日志系统级别

INFO:
一般信息

ERROR:
一般错误  不影响软件继续执行

FATAL:
系统崩溃  core信息

DEBUG:
调试信息

*/
enum loglevel{
    INFO = 0,//普通信息
    ERROR = 1,//普通错误
    FATAL = 2,//core信息
    DEBUG  = 3 //debug信息
};
//使用单例模式  因为一个程序只需要一个日志系统就够了
class  Logger: noncopyable{
    public:
    //函数
    // 用于获取Logger类的实例   
    static int logminlevel_;// 0 1 2 3 
    static Logger& instance();

    //设置日志级别
    void setLogLevel(int level);

    //写日志
    void log(std::string message);
    //变量

    static void LOG_INFO(const char* logmsgFormat, ...);
    static void LOG_ERROR(const char* LogmsgFormat, ...);
    static void LOG_FATAL(const char* LogmsgFormat, ...);
    static void LOG_DEBUG(const char* LogmsgFormat, ...);
    protected:

    private:
    int logLevel_; 
    /**
    INFO,//普通信息 0
    ERROR,//普通错误 1
    FATAL,//core信息 2 
    DEBUG  //debug信息 3
    
    
    
    */
};