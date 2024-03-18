#pragma once 

#include<functional>
#include<unistd.h>
#include<memory>
#include"Socket.h"
#include"EventLoop.h"
#include"noncopyable.h"
#include"Channel.h"
#include"InetAddress.h"
/*
Acceptor 是对accept的封装   主事件循环执行Acceptor

Accepter 只需要处理ReadCallBack  也就是新用户的连接
*/

class Acceptor:noncopyable
{
public:

    using NewConnectionCallback = std::function<void(int sockfd,const InetAddress &inetaddress)>;
    Acceptor(EventLoop *eventloop,const InetAddress &inetaddress,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &callback){newConnectiomCallback_ = callback;};

    bool listenning()const{return listenning_;};
    void listen();
    NewConnectionCallback newConnectiomCallback_;//新连接的回调函数
private:
    //获取新用户连接
    void handleRead();



    EventLoop *loop_;  //Accepter 使用baseloop; 也就是mainloop  mainloop
    Socket acceptSocket_;    
    Channel acceptChannel_;
    // NewConnectionCallback newConnectiomCallback_;//新连接的回调函数
    bool listenning_;


};