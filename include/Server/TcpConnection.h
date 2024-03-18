#pragma once

#include"Channel.h"
#include"Socket.h"
#include"EventLoop.h"
#include"noncopyable.h"
#include"Callbacks.h"
#include"Buffer.h"
#include"Timestamp.h"
#include"Logger.h"

#include<memory>
#include<string>
/*
    Tcpconection 是用户使用的 对用户提供的
    代表已经和服务器建立的连接 的客户端
    数据 : 用户发起连接->  tcpconnection  -> channel  -> poller回调


    Tcpserver  通过 Accepter 监听到有用户连接  Accepter的accept() 函数获得connectfd


*/
/*
    TcpConnection 在subloop中用于执行回调函数  TcpConnect 和 subloop是组合关系
    Accepter 被  mainloop

*/
// public std::enable_shared_from_this<TcpConnection>  执行异步回调函数时 传入shared_from_this()指针 代替this指针  保证不会内存泄漏
class  TcpConnection : noncopyable ,public std::enable_shared_from_this<TcpConnection>
{

public:
    TcpConnection(  EventLoop *loop,
                    const std::string &name,
                    int sockfd,
                    const InetAddress &localaddr,
                    const InetAddress &peeraddr);
                    
    ~TcpConnection();



    //返回当前loop
    EventLoop* getLoop()const {return loop_;};

    const std::string& getName()const {return name_;};
    const InetAddress& getLocaladdr()const {return localaddr_;};
    const InetAddress& getPeeraddr()const {return peerladdr_;};
    //返回是否 已连接
    bool connected()const {return state_.load() == Kdisconnected;};

    void setConnetionCallBack(const ConnetionCallBack &callback) {connectioncallback_  = callback;};

    void setMessageCallBack(const MessageCallBack &callback) {messagecallback_ = callback;};

    void setWriteCompleteCallBack(const WriteCompleteCallBack &callback){writecompletecallback_ = callback;};

    void setHighWaterMarkCallBack(const HighWaterMarkCallBack &callback){highwatermarkcallback_ = callback;};

    void setCloseCallBack(const CloseCallBack &callback) {closecallback_ = callback ;} ;
    
    //发送数据
    void send(const void *message,int len);
    //发送数据
    void send(const std::string &buf);
    
    //关闭连接
    void shutdown();

    //建立连接
    void connectEstablish();
    //销毁连接
    void connectDestroyed();

private:
    enum StateE
    {
        Kdisconnected = 0,//已断开连接
        Kconnecting = 1,//正在连接
        Kconnected = 2,//已连接
        Kdisconnecting = 3//正在断开连接
    };


    EventLoop   *loop_;//提交给subloop 子线程用于执行回调
    const std::string name_ ;//
    std::atomic_int state_;    //网络连接情况
    bool reading_;
    // 网络连接信息 
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    /*
    高水位线计算 :
    每秒n个请求   平均每个请求m个数据量  保留0.1   n*m*1.1



    */
    const size_t highWaterMark_;//高水位线

    const InetAddress localaddr_;//当前主机addr
    const InetAddress peerladdr_;//客户端addr

    /**
    回调函数传递流程 TcpServer ->  TcpConnection  -> Channel 
    poller 监听 channel  然后执行channel 中的回调函数
    最后回调函数都由poller执行 目前poller中内部只支持epoll
    最后由epoll执行

    */
   //5个回调函数
    ConnetionCallBack connectioncallback_;

    MessageCallBack messagecallback_;

    WriteCompleteCallBack  writecompletecallback_;

    HighWaterMarkCallBack highwatermarkcallback_;
    
    CloseCallBack  closecallback_;



    //数据缓冲区
    Buffer inputbuffer_;//接收数据缓冲区


    Buffer outputbuffer_;//发送数据缓冲区
    
    void setStateE(StateE state){
        state_.store(state);
        Logger::LOG_ERROR("TcpConnection.h  setStateE state_ == %d ",state_.load());
        };



    void handleRead(Timestamp time);

    void handleWrite();

    void handleClose();

    void handleError();

    void sendInloop(const void *message,size_t len);
    
    void shutdownInloop();

    void forceCloseInloop();

};