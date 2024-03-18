#include"TcpConnection.h"
#include"TcpServer.h"
#include<functional>
#include"Services.hpp"
#include"Logger.h"
//#include"message.pb.h"
//先使用json测试

class Mainserver{
    public:
    //初始化服务器对象

    Mainserver(EventLoop *loop,
                const InetAddress &hostAddr,
                const std::string name);//3个参数 用于构造 Tcpserver



    void statr(); // 启动服务 也就是启动Tcpserver


    private:
    //mymuduo 的类  
    TcpServer   server_;//
    EventLoop *loop_; //主事件循环 指针
    //当有新连接建立 Tcpserver会回调用这个函数
    
    void onConnection(const TcpConnectionPtr& tcpConnectionPtr); // connectcallback 

    void onMessage(const TcpConnectionPtr& tcpConnectionPtr,Buffer *buff,Timestamp time); // messagecallback 
    //上报读写事件是调用的


};