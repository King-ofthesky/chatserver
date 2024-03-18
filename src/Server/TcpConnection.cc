#include"TcpConnection.h"


#include<functional>
#include<errno.h>
#include<memory>

#include"Logger.h"

//检查lopp是否是空
static EventLoop* checkLoopNotNull(EventLoop *loop){
    if(loop == nullptr){
        //LOG_FATAL()
    }
    return loop;
}

TcpConnection::TcpConnection(  EventLoop *loop,
                                const std::string &name,
                                int sockfd,
                                const InetAddress &localaddr,
                                const InetAddress &peeraddr)
                                :loop_(checkLoopNotNull(loop)),
    name_(name),
    reading_(true),
    socket_(new Socket(sockfd)),
    state_(Kconnecting),
    channel_(new Channel(loop_,sockfd)),
    localaddr_(localaddr),
    peerladdr_(peeraddr),
    highWaterMark_(1024*1024*64)//64M   
{
    setStateE(Kconnecting);
    /*设置channel_的回调函数  对应epoll_ctl中所有的事件 
    */
    Logger::LOG_INFO("TcpConnection.cc  TcpConnection state_ == %d ",state_.load());
    Logger::LOG_INFO("TcpConnection.cc 创建 TcpConnection");
    channel_->setReadCallBack(
        std::bind(&TcpConnection::handleRead,this,std::placeholders::_1)
        );

    channel_->setWriteCallBack(
        std::bind(&TcpConnection::handleWrite,this)
    );

    channel_->setErrorCallBack(
        std::bind(&TcpConnection::handleError,this)
    );

    channel_->setCloseCallBack(
        std::bind(&TcpConnection::handleClose,this)
    );

    //启动保持活动状态的操作。当这个选项被启用时，套接字会定期发送心跳包以保持连接的活跃性
    socket_->setKeepAlive(true);
    Logger::LOG_INFO("TcpConnection.cc 创建 TcpConnection   ok");
            
}



//析构函数调用意味着连接断开
TcpConnection::~TcpConnection(){
    Logger::LOG_INFO("TcpConnection.cc 客户端连接断开");
}




/* 发送数据
    如果应用写的快 内核发送慢  则需要写入缓冲器   const void *message  是 buffer.c_str()
*/
void TcpConnection::sendInloop(const void *message,size_t len)
{
    Logger::LOG_INFO("TcpConnection.cc sendInloop  在当前loop发送数据 ");
    ssize_t writelen = 0;  //已发送数据
    ssize_t pendinglen = len;//未发送数据长度
    bool faultError = false;
    if(state_.load() == Kdisconnected)//连接断开 无法发送
    {
        Logger::LOG_ERROR("TcpConnection.cc sendInloop connect has disconnected cannot send");
    }
    
    //  channel 不可写  并且  缓冲器没有发送数据   第一次写入
    if(!channel_->isWritingEvent() && (outputbuffer_.readableBytes() == 0) ) 
    {
        
        writelen = ::write(channel_->fd(),message,len);
        if(writelen >= 0)  // 数据发送成功
        {
            Logger::LOG_INFO("TcpConnection.cc sendInloop 数据发送成功");
            pendinglen  =  len - writelen;
            if(pendinglen == 0 )//  数据一次发送完
            {
                if(writecompletecallback_)
                {
                    loop_->queueInLoop(
                        //异步回调操作  使用shared_from_this
                        std::bind(&TcpConnection::writecompletecallback_,shared_from_this())
                    );
                }
            }
        }
        else  // writelen <0  写入数据出错
        {
           writelen = 0;
           if(errno != EWOULDBLOCK)  // 非阻塞操作将会阻塞的错误码  出错  系统内部阻塞或者 对端关闭端口  或者连接重置
            {
                Logger::LOG_ERROR("EWOULDBLOCK");
                if(errno == EPIPE || errno == ECONNRESET)//连接被重置   或者 对方关闭端口
                {
                    Logger::LOG_ERROR("EPIPE || ECONNRESET");
                }
                    faultError = true;// 因为对面端口问题而发生错误  不是因为自己系统阻塞   
            }
        }
    }
    //连接正常  执行了一次write  数据一次没发完 再次发送
    //执行channel->readcallback  ==  TcpConnect->handlewrite
    //将发送缓冲区数据全部发送完 
    if(faultError == false && pendinglen > 0) 
    {
        size_t  olen = outputbuffer_.writeableBytes(); //发送缓冲器剩余需要发送数据
        if(olen + pendinglen >= highWaterMark_ && olen <highWaterMark_ && highwatermarkcallback_)  
        //总发送数据长度 大于高水位 
        {   
            //水位线回调
            loop_->queueInLoop(std::bind(
                highwatermarkcallback_,shared_from_this(),olen+pendinglen
            )
            );
        }
        //第一次没写入成功就把剩下的数据追加到buff中去
        //让outputbuffer_ 去自动写入数据
        outputbuffer_.append((char*)message+writelen,pendinglen);
        
        //第一次发送数据  如果没发完  需要注册channel写事件 不然buff不发数据
        if(!channel_->isWritingEvent())
        {
            //注册写事件 否则epoll不会通知epollout  不然不会执行writeEventCallBack_
            channel_->enablewriting();
        }
    }

}

/* 封装 sendinloop  判断当前线程能不能执行sendinloop*/
void TcpConnection::send(const std::string &buf){
    Logger::LOG_ERROR("TcpConnection.cc send ");
    if(state_.load()== Kconnected)//如果连接还在  就发送数据
    {
    Logger::LOG_ERROR("TcpConnection.cc send 连接在判断是否当前loop ");
        /*
            同一进程的所有线程共享堆空间 fd 和静态变量  线程独享栈空间  buff是在栈空间的  loop执行操作是 先判断是否是当前线程
            否则栈上没数据会出错
        */
        if(loop_->isInLoopThread())  //判断当前线程是不是这个loop所属
        {
            Logger::LOG_ERROR("TcpConnection.cc send 在当前loop  执行sendInloop ");
            sendInloop(buf.c_str(),buf.length());//发送数据
        }
        else//不在这个线程
        {
            //如果loop不在当前线程    把回掉操作放到放到 回调函数队列中  //异步回调操作  使用shared_from_this
            Logger::LOG_ERROR("TcpConnection.cc send 不在当前loop  queueInLoop");
            loop_->queueInLoop(std::bind(
                &TcpConnection::sendInloop,
                shared_from_this(),
                buf.c_str(),
                buf.length()
            ));
        }
    }
    else
    {
            Logger::LOG_ERROR("TcpConnection.cc send error 连接已经断开 state_ == %d ",state_.load());
    }
}









//读数据发生
void TcpConnection::handleRead(Timestamp time)
{
    Logger::LOG_INFO("TcpConnection.cc handleRead 执行handleRead");
    int erro = 0;
    ssize_t n = inputbuffer_.readfd(channel_->fd(),&erro);//socket_.fd()
    if(n > 0)   //连接的用户有可以读的数据  
    {
        Logger::LOG_INFO("TcpConnection.cc handleRead 有数据可读调用messagecallback");
        //调用用户的回调函数
        // std::shared_from_this()  == 当前Tcpconnection 对象的智能指针   
        //Tcpconnection 对象继承了 std::enable_shared_from_this<TcpConnection>
        
        messagecallback_(shared_from_this(),&inputbuffer_,time);
        //messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0) // 没读到数据 客户端断开连接
    {
        //执行断开回调函数
        Logger::LOG_INFO("TcpConnection.cc handleRead 没读到数据 客户端断开连接");
        handleClose();
    }
    else  //  n < 0  出错  
    {
        //保存错误代码
        erro  = errno ;
        Logger::LOG_ERROR("TcpConnection.cc handleRead errno %d",erro);
        //LOG_ERROR("errno %d",erro);
        handleError();
    }
}

// epollout
void TcpConnection::handleWrite()
{
    //
    int erro  = 0;
    if(channel_->isWritingEvent())  //判断是否可以写  
    {   
        //向写缓冲器写入数据
        ssize_t n = outputbuffer_.writefd(channel_->fd(),&erro); 
        if(n >0)
        {
            outputbuffer_.retrieve(n);  //写入数据后复位缓冲区
            if(outputbuffer_.readableBytes() == 0)//发送完成
            {
                channel_->disablewriting();
                if(writecompletecallback_)//有回调函数执行回调函数
                {
                    // 唤醒当前loop对应的thread 执行回调函数   也是当前tcpconnection所在的线程
                    //自己写错了 应该bind
                    // loop_->queueInLoop(writecompletecallback_,shared_from_this());
                    loop_->queueInLoop(std::bind(writecompletecallback_,shared_from_this()));
                }
                if(state_.load() == Kdisconnecting)//正在关闭 服务器  用户执行了TcpConnection::shutdown函数
                {
                    shutdownInloop();//在当前loop 删除tcpconnection
                }

            }
        }
        else  //写输出出错   
        {

           // LOG_ERROR();
        }
    }
    else //channel 不可写数据
    {
        // LOG_ERROR("CONNCET fd=%d  not  more write ",channel->fd());
    }

}

void TcpConnection::handleClose()
{
    Logger::LOG_INFO("TcpConnection.cc handleClose fd= %d state = %d\n ",channel_->fd(),state_.load());//关闭后写入日志信息
    setStateE(Kdisconnected);//设置关闭状态
    channel_->disableAll();//设置epoll中所有事件 都不感兴趣


    TcpConnectionPtr guard(shared_from_this());

    if(connectioncallback_)
    {
        connectioncallback_(guard);
    }


    //执行关闭链接回调
    if(closecallback_)
    {
        closecallback_(guard);
    }

}

//对error的回调执行
void TcpConnection::handleError()
{
    int opt;
    socklen_t  optlen  = sizeof(opt);
    int erro = 0;
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&opt,&optlen)<0)
    {
        erro =errno;

    }
    else
    {
        erro  = opt;
    }
    //LOG_ERROR("Tcpconnctio::handleError name %s -so_error ^d \n ",name_.c_str(),erro);
}


//建立连接
void TcpConnection::connectEstablish()
{
    setStateE(Kconnected);
    //TcpConnection 管理了channel  所以channel需要保存一个TcpConnection的一个智能指针
    // 如果TCPconnecttion被remove了 channel在执行回调函数时 Tcpconnection对象还存在 
    //通过
    //
    channel_->tie(shared_from_this());
    channel_->enableReading();//向epoll注册epollin事件

    //有新链接建立回调
    if(connectioncallback_){
        connectioncallback_(shared_from_this());
    }

}
//销毁连接


void TcpConnection::connectDestroyed()
{
    if(state_.load() == Kconnected)//连接已经建立了 才可以关闭
    {
        Logger::LOG_INFO("TcpConnection.cc connectDestroyed fd= %d state = %d\n ",channel_->fd(),state_.load());//关闭后写入日志信息
        setStateE(Kdisconnected);//设置连接状态关闭
        channel_->disableAll();//epoll中channel所有感兴趣事件都删除 ::epoll_ctl(epoll_fd_,operation,fd,&event)
        if(closecallback_)
        {
            closecallback_(shared_from_this());
        }
        channel_->remove();//把channel从poll中删除
    }
}

//关闭连接
void TcpConnection::shutdown()
{
    if(state_.load() == Kconnected)//判断状态是否是 已连接
    {
        setStateE(Kdisconnecting);//设置状态为  断开连接中
        loop_->runInLoop(std::bind(    //在当前loop中执行  TcpConnection的shutdownINloop回调函数    TcpConnection是用户
            &TcpConnection::shutdownInloop,this
        ));
    }
}

void TcpConnection::shutdownInloop()
{
    if(!channel_->isWritingEvent())  //说明 outputbuffer  数据发生完毕  没有写事件
    {
        socket_->shutdownWrite();//关闭写端  触发EpollHUP
    }

}