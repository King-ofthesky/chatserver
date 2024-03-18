#pragma once
#include<arpa/inet.h>
#include<netinet/in.h>
// #include<sys/socket.h>
#include<string>
#include<strings.h>
#include<string.h>
#include"copyable.h"

class InetAddress:public copyable{
    public:

    explicit InetAddress(u_int16_t port = 0 ,std::string ip = "127.0.0.1");

    explicit InetAddress(const sockaddr_in &addrin):addr_(addrin){};

    std::string toIp() const;

    u_int16_t toPort() const;  // 16位 无符号整数 0~65535

    std::string toIpPort() const;

    const sockaddr* getSockaddr() const{return (sockaddr *)&addr_ ;};

    void setSockaddr(const sockaddr_in &sockaddr){addr_ = sockaddr;};
    protected:

    private:
    // union{
    //     struct sockaddr_in  addr_;
    //     struct sockaddr_in6 addr6_;
    // };
    struct sockaddr_in  addr_;
};
