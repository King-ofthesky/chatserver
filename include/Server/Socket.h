#pragma once

#include"noncopyable.h"
#include"InetAddress.h"
#include <cerrno>
#include<iostream>
class InetAddress;
class Socket:noncopyable{

public:
    explicit Socket(int fd)
        :sockfd_(fd)
    {
        std::cout<<"Socket.h14 SOCKET 对象创建成功"<<std::endl;
    }

    ~Socket();
    int fd()const{return sockfd_;};
    void bindInetAddress(const InetAddress inetaddress);
    void listen();
    int accept(InetAddress *peeraddr);


    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};