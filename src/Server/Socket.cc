#include"Socket.h"
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/tcp.h>
#include"Logger.h"

Socket::~Socket()
{

}

void Socket::bindInetAddress(const InetAddress inetaddress)  //绑定 socket  目前支持ipv4
{
    if(0!=::bind(sockfd_,(struct sockaddr *)inetaddress.getSockaddr(),sizeof(sockaddr_in)))
    {
        // std::cout<<"Socket.cc  bind失败"<<std::endl;
        // std::cout<<inetaddress.toIpPort()<<std::endl;

        std::string err = "Socket.cc  bind失败" + inetaddress.toIpPort();
        Logger::LOG_ERROR(err.c_str());
    }
}

void Socket::listen()
{
    Logger::LOG_INFO("Socket.cc  执行listen  开启监听");
    std::cout<<"Socket.cc  socket_值为  "<<std::to_string(sockfd_)<<std::endl;
    if(::listen(sockfd_,SOMAXCONN)==0)
    {
        // std::cout<<"Socket.cc  listen成功"<<std::endl;
        Logger::LOG_INFO("Socket.cc  listen成功  开启监听");
    }else
    {
        Logger::LOG_ERROR("Socket.cc  listen失败");
    }
}
int Socket::accept(InetAddress *peeraddr)
{
    std::cout << "Socket.cc  accept 正在执行" << std::endl;
    sockaddr_in addr; //ipv4
    socklen_t len = sizeof(addr); 
    // bzero(&addr,sizeof(addr));
    memset(&addr, 0, sizeof(addr));
    int connfd = ::accept4(sockfd_,(sockaddr*)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    int savedErrno = errno;
    if(connfd>=0){
        std::cout << "Socket.cc  setSockaddr" << std::endl;
        peeraddr->setSockaddr(addr);
        std::cout << "Socket.cc  setSockaddr   ok" << std::endl;
    } else {
        if (savedErrno == EAGAIN) {
            std::cout << "Socket.cc Accept would block" << std::endl;
        } else if (savedErrno == ECONNABORTED) {
            std::cout << "Socket.cc Connection aborted" << std::endl;
        } else if (savedErrno == EMFILE) {
            std::cout << "Socket.cc Per-process limit of open file descriptors reached" << std::endl;
        } else {
            std::cout << "Socket.cc Error in accept: " << strerror(savedErrno) << std::endl;
        }
    }
    return connfd;
}
//关闭写端 触发EPOLLHUP
void Socket::shutdownWrite()  
{
    if(::shutdown(sockfd_,SHUT_WR)){
            //ERROR()
    }
}

void Socket::setTcpNoDelay(bool on)
{//协议级别
    int opt = on ? 1 :0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&opt,sizeof(opt));
}

void Socket::setReuseAddr(bool on)
{//socket级别
    int opt = on ? 1 :0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
}

void Socket::setReusePort(bool on)
{
    int opt = on ? 1 :0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));
}

void Socket::setKeepAlive(bool on)
{
    int opt = on ? 1 :0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&opt,sizeof(opt));
}

