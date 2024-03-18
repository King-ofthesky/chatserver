#include"Acceptor.h"
#include"Logger.h"

static int createNonblocking()    //创建一个非阻塞fd
{
    int sockfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
    if(sockfd<0){
        Logger::LOG_INFO("Acceptor.cc 创建acceptor sockfd 失败");
    }else{
        Logger::LOG_INFO("Acceptor.cc 创建acceptor sockfd 成功");
        return sockfd;
    }
    return -1;
}
//
Acceptor::Acceptor(EventLoop *eventloop,const InetAddress &inetaddress,bool reuseport)
            :loop_(eventloop),
            acceptSocket_(createNonblocking()),
            acceptChannel_(loop_,acceptSocket_.fd()),
            listenning_(true)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindInetAddress(inetaddress);//bind  函数
    Logger::LOG_INFO("Acceptor.cc setReadCallBack");
    acceptChannel_.setReadCallBack(std::bind(&Acceptor::handleRead,this));//accept()  函数  newConnection 函数
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    Logger::LOG_INFO("Acceptor.cc 主线程执行listen");
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    Logger::LOG_INFO("Acceptor.cc 设置acceptChannel_为enableReading 状态");
}
void Acceptor::handleRead() //获取新用户连接
{
    Logger::LOG_INFO("正在执行accept 等待客户端连接");
    InetAddress peerAddr;//客户端地址
    int connfd = acceptSocket_.accept(&peerAddr);//通过accept函数 获取一个连接的客户端ip和port和fd  
    if(connfd >=0)  //有链接
    { 
        Logger::LOG_INFO("Acceptor.cc  有客户端连接 fd为%d  ip   %s",connfd,peerAddr.toIpPort().c_str());

        if(newConnectiomCallback_){   //当获取一个链接有 尝试执行回调函数

            Logger::LOG_INFO("执行客户端新连接回调函数");
            newConnectiomCallback_(connfd,peerAddr);//执行建立新连接的回调函数
        }else{

            Logger::LOG_INFO("Acceptor.cc  handleRead 没有有newnewConnectiomCallback_  结束");
            ::close(connfd);//没有回调函数直接结束
        }
    }
    else{//
        //LOG_ERROR()
        if(errno = EMFILE ){//fd到上限

            Logger::LOG_ERROR("fd达到上限");

        }
    }
}