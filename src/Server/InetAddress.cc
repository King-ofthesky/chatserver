#include"InetAddress.h"

InetAddress::InetAddress(u_int16_t port,std::string ip){
    bzero(&addr_,sizeof(addr_));//清0
    addr_.sin_family = AF_INET;  //ipv4 地址族
    addr_.sin_port = htons(port);  // host 转 net 字节序  short类型  16位  网络字节序 大端
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());  
}

std::string InetAddress::toIp() const{
    char buf[64] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr,buf,64);
    return  buf;
}

u_int16_t InetAddress::toPort() const{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const{
    char buf[64] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr,buf,64);
    size_t len = strlen(buf);
    u_int16_t port = ntohs(addr_.sin_port);
    sprintf(buf+len,":%u",port);
    return  buf;
}