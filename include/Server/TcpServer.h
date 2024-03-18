#pragma once 

#include<functional>
#include<string>
#include<memory>
#include<mutex> 
#include<atomic>
#include<unordered_map>

#include"Logger.h"
#include"EventLoopThreadPoll.h"
#include"EventLoop.h"
#include"noncopyable.h"
#include"InetAddress.h"
#include"Acceptor.h"
#include"TcpConnection.h"
#include"Buffer.h"
#include"Timestamp.h"
#include"Callbacks.h"
//对外编程使用的类
//用户使用的类
class TcpServer{

public:
 
    using ThreadInitCallBack = std::function<void(EventLoop*)>;
    using ConnectMap = std::unordered_map<std::string,TcpConnectionPtr>;
    //端口是否使用枚举
    enum Option{
        KNoReusePort, //端口没被使用  
        KReusePort,//端口使用
    };

    TcpServer(EventLoop *loop,
                const InetAddress &hostAddr,
                const std::string name,
                Option option = KReusePort);
    
    ~TcpServer();

    void setThreadInitCallBack(const ThreadInitCallBack &callback){threadInitCallBack_ = callback;};
    void setConnetionCallBack(const ConnetionCallBack &callback){connetionCallBack_ = callback;};
    void setMessageCallBack(const MessageCallBack &callback){messageCallBack_ = callback;};
    void setWriteCompleteCallBack(const WriteCompleteCallBack &callback){writeCompletaCallBack_ = callback;};

    void setThreadNumber(int numThreads);//这只Thread数目

    //开启服务器监听 
    void start();







    // using TcpConnectionPtr =  std::shared_ptr<TcpConnection>;
    // using ConnetionCallBack = std::function<void(const TcpConnection&)>;
    // using WriteCompleteCallBack = std::function<void(const TcpConnection&)>;
    // using HighWaterMarkCallBack = std::function<void(const TcpConnection&)>;
    // using MessageCallBack = std::function<void(const TcpConnection&, Buffer*, Timestamp)>;
private:
    EventLoop *loop_;//baseloop  用户定义的
    std::string name_; // 服务器主线程 名字
    std::string ipPort_;  //开启的端口

    std::unique_ptr<Acceptor>  acceptor_;//mainloop监听  
    std::shared_ptr<EventLoopThreadPoll> ThreadPoll_; 


    //InitThreadCallBack initThreadCallBack_;  
    ConnetionCallBack connetionCallBack_;//监听到新连接到来时调用
    MessageCallBack messageCallBack_;//已连接用户有读写事件发生时调用
    WriteCompleteCallBack writeCompletaCallBack_;//写数据  数据写完之后调用
    ThreadInitCallBack threadInitCallBack_;//loop_线程初始化的回调
    
    std::atomic_int started_;
    
    int nextConnId_;
    ConnectMap   connects_;//保存所有的连接

    void newConnection(int fd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);    
};