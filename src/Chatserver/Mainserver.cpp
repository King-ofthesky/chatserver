#include"Mainserver.hpp"

Mainserver::Mainserver(EventLoop *loop,
                const InetAddress &hostAddr,
                const std::string name)
                :server_(TcpServer(loop,hostAddr,name))
                ,loop_(loop)
{

    //注册回调函数
    //用绑定器 绑定新链接时的回调函数 
    Logger::LOG_INFO(" 创建mainserver ");
    server_ .setConnetionCallBack(std::bind(&Mainserver::onConnection,this,std::placeholders::_1));

    server_.setMessageCallBack(std::bind(&Mainserver::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    //设置启动线程数
    server_.setThreadNumber(3);//
    
}

void Mainserver::statr()
{
    server_.start();
    Logger::LOG_INFO("Mainserver.cpp Mainserver statr  ok");
}

//注册到 tcpserver 的回调函数  当与客户端建立tcp连接时 调用此回调函数
void Mainserver::onConnection(const TcpConnectionPtr& tcpConnectionPtr)
{
    //每次有连接建立时 调用此回调函数
    Logger::LOG_INFO("Mainserver.cpp onConnection  do ");
    Logger::LOG_INFO("Mainserver.cpp onConnection  执行onconnection客户端的ip为 %s",tcpConnectionPtr->getPeeraddr().toIpPort().c_str());
}


//在与客户端建立连接后 当客户端发送的数据到达时

/**
 * 表示与客户端建立的TCP连接，通过它可以进行数据的发送和关闭连接等操作。
 * conn：表示与客户端建立的TCP连接，通过它可以进行数据的发送和关闭连接等操作。
 * buffer：表示接收到的数据存储的缓冲区指针，您可以使用该缓冲区读取和处理数据。
 * receiveTime：表示接收到数据的时间戳。
*/
void Mainserver::onMessage(const TcpConnectionPtr& tcpConnectionPtr,Buffer *buff,Timestamp time)
{
    Logger::LOG_INFO("Mainserver.cpp onMessage  do ");
    // 当有消息到达的时候 会调用onMEssage函数  然后我们在这个函数中根据id执行对应的函数
    //读取缓冲区所有数据 然后保存到string中; 
    //拷贝构造函数  从*buff读取数据到buf 一共拷贝2次
    //第一次 在buff内部定义一个栈string 存储 std::string result(peek(),len);
    //然后返回到此函数时再进行一次string拷贝构造函数调用
    Logger::LOG_INFO("Mainserver.cpp onMessage  读取buff ");
    std::string buf = buff->retrieveAllAsstring(); //拷贝构造函数调用
    Logger::LOG_INFO("Mainserver.cpp onMessage  读取buff   size为 %d",buf.size());
    Logger::LOG_INFO("Mainserver.cpp onMessage  读取buff   ok  读取内容为 %s",buf.c_str());
    Logger::LOG_INFO("Mainserver.cpp onMessage  执行send ");
    tcpConnectionPtr->send(buf);
    std::cout<<buf<<std::endl;
    // tcpConnectionPtr->shutdown();//触发epollhup    执行closecallback

    /*
    性能优化版 直接从buff的 char* 指针读取数据
    std::string buf2(buff->peek(),buff->readableBytes());//有缺点 虽然少一次数据复制 但是需要手动调用
    buff->retrieve(buff->readableBytes()); 
    */
    // Logger::LOG_INFO(__FILE__,__LINE__,buf.c_str());

    // 创建json 对象
    // json js = json::parse(buf);
    // int id = js["messageid"].get<int>();//得到id这个参数
    // //通过单例Services对象 获取对应消息的回调函数
    // messageHandle messfunc  = Services::instance()->gethandle(id);
    // //最关键的一步 通过对应的回调函数执行对应的操作  不用去switch case
    // messfunc(tcpConnectionPtr,js,time);


    // LogRequestProto logmsg;

    //  if (!(logmsg.ParseFromArray(buff->peek(), buff->readableBytes())))
    // {
    //     //序列化失败的代码
    // }
    //从probuf得到id 然后从map中得到回调函数容纳后执行
    // messageHandle msghandle  = nullptr;
    // Services.instance()->gethandle(logmsg.ptrid());
    // msghandle(tcpConnectionPtr,buff,time);
}