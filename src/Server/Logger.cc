#include"Logger.h"
#include<cstdarg>
/*
    使用宏不知道为啥编译失败 全部用函数代替
*/
// #define LOG_INFO(logmsgFormat,...)\
//     do\
//     {\
//     Logger &logger =  Logger::instance();\
//     logger.setLogLevel(INFO);\
//     char buf[1024] = {0};\
//     snprintf(buf,1024,logmsgFormat,##__VA_ARGS__);\
//     logger.log(buf);\
//     }while(0)

// #define LOG_ERROR(LogmsgFormat,...)\
//     do\
//     {\
//     Logger &logger =  Logger::instance();\
//     logger.setLogLevel(ERROR);\
//     char buf[1024] = {0};\
//     snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__);\
//     logger.log(buf);\
//     }while(0)

// #define LOG_FATAL(LogmsgFormat,...)\
//     do\
//     {\
//     Logger &logger =  Logger::instance();\
//     logger.setLogLevel(FATAL);\
//     char buf[1024] = {0};\
//     snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__);\
//     logger.log(buf);\
//     }while(0)

// #ifdef MODUODEBUG
// #define LOG_DEBUG(LogmsgFormat,...)\
//     do\
//     {\
//     Logger &logger =  Logger::instance();\
//     logger.setLogLevel(DEBUG);\
//     char buf[1024] = {0};\
//     snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__);\
//     logger.log(buf);\
//     }while(0)
// #else
// #define LOG_DEBUG(LogmsgFormat,...)
// #endif
int Logger::logminlevel_ = 0;

void Logger::LOG_INFO(const char* logmsgFormat, ...) {
    if(INFO <logminlevel_){ //控制日志系统是否打印某些级别日志
        return; 
    }
    Logger& logger = Logger::instance();
    logger.setLogLevel(INFO);


    char buf[1024] = {0};

    va_list args;
    va_start(args, logmsgFormat);
    vsnprintf(buf, 1024, logmsgFormat, args);
    va_end(args);

    logger.log(buf);
}


void Logger::LOG_ERROR(const char* LogmsgFormat, ...) {
    if(ERROR < logminlevel_){        //控制日志系统是否打印某些级别日志
    return; 

    }
    Logger& logger = Logger::instance();
    logger.setLogLevel(ERROR);

    char buf[1024] = {0};
    
    va_list args;
    va_start(args, LogmsgFormat);
    vsnprintf(buf, 1024, LogmsgFormat, args);
    va_end(args);

    logger.log(buf);
}

void Logger::LOG_FATAL(const char* LogmsgFormat, ...) {
    if(FATAL < logminlevel_){    //控制日志系统是否打印某些级别日志
        return;

    }
    Logger& logger = Logger::instance();
    logger.setLogLevel(FATAL);

    char buf[1024] = {0};

    va_list args;
    va_start(args, LogmsgFormat);
    vsnprintf(buf, 1024, LogmsgFormat, args);
    va_end(args);

    logger.log(buf);
}
void Logger::LOG_DEBUG(const char* LogmsgFormat, ...) {
    if(DEBUG <logminlevel_){    //控制日志系统是否打印某些级别日志
        return;
    }
    Logger& logger = Logger::instance();
    logger.setLogLevel(DEBUG);

    char buf[1024] = {0};

    va_list args;
    va_start(args, LogmsgFormat);
    vsnprintf(buf, 1024, LogmsgFormat, args);
    va_end(args);

    logger.log(buf);
}

// 用于获取Logger类的实例   
Logger& Logger::instance(){
    static Logger logger;
    return logger;
}

//设置日志级别
void Logger::setLogLevel(int level){
    logLevel_ = level;
}
//写日志  级别信息  + 时间 : 信息
void Logger::log(std::string message){
    //打印 类型
    switch (logLevel_)
    {
    case INFO:
        std::cout <<"INFO  " ;
        break;
    case ERROR:
        std::cout <<"ERROR  " ;
        break;
    case FATAL:
        std::cout <<"FATAL  " ;
        break;
    case DEBUG:
        std::cout <<"DEBUG  " ;
        break;
    default:
        break;
    }
    //打印时间

    //打印message

    std::cout<< Timestamp::now().toString()<<"  "<<message <<std::endl;

}