#include"TcpServer.h"
#include"Logger.h"
#include<functional>
EventLoop* CheckNotNull(EventLoop *loop)
{
    if(loop == nullptr){
        // LOG_FATAL("%s:%s:%d mainloop is null \n",__FILE__, __FUNCTION__ ,__TIME__ );
    }
    return loop;
}


/*
创建一个TcpServer 意味着


*/
TcpServer::TcpServer(EventLoop *loop,
                    const InetAddress &hostAddr,
                    const std::string name, 
                    Option option )
                    :loop_(CheckNotNull(loop))
                    ,ipPort_(hostAddr.toIpPort())
                    ,name_(name)
                    ,acceptor_(new Acceptor(loop,hostAddr,option == KReusePort))  //创建mianloop
                    ,ThreadPoll_(new EventLoopThreadPoll(loop_,name_))//创建threadpool  传入mainloop  默认只有主线程
                    ,connetionCallBack_() 
                    ,messageCallBack_()
                    ,nextConnId_(1)
                    ,started_(0)
{
    if(acceptor_){
        Logger::LOG_INFO( "TcpServer.cc acceptor_  new 成功 ");
    }else{
        Logger::LOG_ERROR( "TcpServer.cc acceptor_  为空  ");
    }
    Logger::LOG_INFO( "设置setNewConnectionCallback   ");
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
    if(acceptor_->newConnectiomCallback_)
    {
        Logger::LOG_INFO("设置setNewConnectionCallback  ok");
    }else{
        Logger::LOG_INFO("设置setNewConnectionCallback  fail");
    }

}

TcpServer::~TcpServer()
{
    //std::unordered_map<std::string,TcpConnectionPtr>
    //using TcpConnectionPtr =  std::shared_ptr<TcpConnection>
    for(auto &con: connects_){
        TcpConnectionPtr p(con.second);//短暂持有智能指针
        con.second.reset();//TcpConnectionPtr std::shared_ptr.reset   //释放智能指针  最后出当前循环后智能指针会调用析构函数
        //销毁连接
        p->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed,p)
        );
    }
}

void TcpServer::setThreadNumber(int numThreads)//这只Thread数目
{
    ThreadPoll_->setThreadNum(numThreads);
}
//开启服务器监听 

/*
包括 启动线程池  和启动acceptor监听
*/
void TcpServer::start()
{
    std::cout<<"TcpServer.cc   start"<<std::endl;
    // std::cout<<"TcpServer.cc"<< "started_---" << started_ <<std::endl;
    if(started_++ == 0)//防止多次启动
    {
        ThreadPoll_->start(threadInitCallBack_); //启动线程池  
        Logger::LOG_INFO("TcpServer.cc 线程池启动成功");
        //
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));//当前主线程执行listen函数
        Logger::LOG_INFO("TcpServer.cc 主线程 runInLoop 执行完毕");
        /*
        ThreadPoll_->start(threadInitCallBack_); //启动线程池   
        根据threadInitCallBack_ 变量创建n个线程ThreadPoll_
        std::vector<std::unique_ptr<EventLoopThread>> threads_;//所有创建的线程
        std::vector<EventLoop*> eventloops_;//所有eventloop的指针
        在ThreadPoll_中2个变量分别存储了线程 和eventloop
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));//当前主线程执行listen函数

        这个函数本质上是在当前线程 也就是mainloop启动了
        (::listen(sockfd_,SOMAXCONN)==0)
        然后给sockchanel 设置了可读事件
        */
    }else{

    }
}

/*
//新链接回调函数  的函数这个函数会给 Acceptot 当有新链接到来时  Acceptor会执行此回调函数
*/
void TcpServer::newConnection(int fd,const InetAddress &peerAddr)
{
    Logger::LOG_INFO("TcpServer.cc  newConnection do");
    EventLoop *subloop = ThreadPoll_->getNextLoop();//从线程池中选中一个线程  然后把当前连接交给这个线程去管理
    char buf[64] = {0};
    snprintf(buf,64,"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string name = name_+buf; //新连接的名字  

    Logger::LOG_INFO("TcpServer.cc newConnection  name: %s",name.c_str());
    //通过 socket
    sockaddr_in localin;
    bzero(&localin,sizeof(localin));
    socklen_t addrlen  = sizeof(localin);
    if(::getsockname(fd, (sockaddr*)&localin,&addrlen) <0  ){  //根据fd获得 sockaddr_in  也就是ipv4  port
        Logger::LOG_ERROR("TcpServer.cc  newConnection getsockname失败");
    }
    Logger::LOG_INFO("TcpServer.cc newConnection  ::getsockname(fd, (sockaddr*)&localin,&addrlen) ");
    InetAddress localaddr(localin);
    TcpConnectionPtr connptr(new TcpConnection(
            subloop,
            name,
            fd,// 当前连接
            localaddr,
            peerAddr
    ));
    //把新链接放到connects_ map中
    Logger::LOG_INFO("TcpServer.cc  newConnection connects_ 加入新链接");
    connects_[name] = connptr; 

    //设置回调函数
    /*
        这些回调函数都是用户设置的   最终被channel调用   
    */
    connptr->setConnetionCallBack(connetionCallBack_);

    connptr->setMessageCallBack(messageCallBack_);

    connptr->setWriteCompleteCallBack(writeCompletaCallBack_);

    connptr->setCloseCallBack(
        std::bind(&TcpServer::removeConnection,this,std::placeholders::_1)
    ); 
    //只有有客户端连接 就会直接调用 Tcpconnect的connectEstablish方法  向epoll中注册epollin 事件
    subloop->runInLoop(std::bind(
        &TcpConnection::connectEstablish,connptr
    ));
    
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop,this,conn)
    );
        
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    Logger::LOG_INFO("Tcpserver removeconnect  name [%s] connection %s\n",name_.c_str(),conn->getName().c_str());

    ssize_t n = connects_.erase(conn->getName());//从map中删除 TcpConnect对象
    EventLoop *subloop = conn->getLoop();//得到TcpConnect所属的loop的指针
    //让所属loop执行 这个TcpConnect 的回调函数 

    /*
    channel ->closecallback    == TcpConnection::handleClose
    */
    subloop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn)
    );
    

}  