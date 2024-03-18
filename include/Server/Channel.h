#pragma  once 

#include<functional>
#include<memory>
#include<assert.h>

#include"Logger.h"
#include"Timestamp.h"
#include"noncopyable.h"
#include"EventLoop.h"
#include<sys/epoll.h>

//对象前置声明  只使用指针
class EventLoop;
class Timestamp;

/*通道  channel 封装了sockfd    和 感兴趣的 event, 包括 EPOLLIN  EPOLLOUT事件 和所有事件处理的函数
        channel 持有一个eventLoop指针  这个指针是当前channel所在loop对象的指针;

channel 知道




*/
class Channel:noncopyable{
    public:

    //回调函数  
    using EventCallBack =  std::function<void()>  ; 
    //只读事件回调函数
    using ReadEventCallBack =  std::function<void(Timestamp)>; 

    Channel(EventLoop *Loop,int fd);

    ~Channel();


    void handleEvent(Timestamp receiveTime);
    void handleEventWithGuard(Timestamp receiveTime);

    int fd() const{return fd_;};
    int index(){return index_;};
    void setIndex(int index){index_ = index;};


    //返回fd当前的事件状态
    //没有事件
    bool isNoneEvent()const {return events_ == kNoneEvent;};  
    //有写事件  要发送数据
    bool isWritingEvent()const{return events_ & KwriteEvent;};  
    //有读事件  接收到数据
    bool isReadingEvent()const {return events_ & kReadEvent;};  




    //private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int KwriteEvent;

    EventLoop *loop_;//事件循环

    const int fd_;  // Poller 监听的对象
    int events_;    // 注册fd 感兴趣的事件
    int revents_;   //Poller 返回的 具体发生的 事件
    int index_;

    std::weak_ptr<void> tie_;//智能指针 跨线程监听
  
    bool tied_;//tie是否绑定 绑定后tied 置为true;   绑定的是TcpConnection对象  也有可能是Accept对象

    bool eventHandling_;
    bool addedToEventLoop_;

    //回调函数   
    ReadEventCallBack readEventCallBack_;//void(Timestamp)
    EventCallBack  writeEventCallBack_; //void()
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
    
    //设置回调函数  

    //通过移动语义  转义所有权  
    void setReadCallBack(ReadEventCallBack cb){
       
        readEventCallBack_ = std::move(cb);
        if(readEventCallBack_){
            Logger::LOG_INFO("Channel.h  setReadCallBack设置成功");
        }else{
            Logger::LOG_INFO("Channel.h  setReadCallBack设置失败");
        }

        
        };

    void setWriteCallBack(EventCallBack cb){writeEventCallBack_= std::move(cb);};
    void setCloseCallBack(EventCallBack cb){closeCallBack_ = std::move(cb);};
    void setErrorCallBack(EventCallBack cb){errorCallBack_ = std::move(cb);};
    
    //防止channel被手动remove之后,channel还继续执行回调函数
    //判断channel 是否被remove
    void tie(const std::shared_ptr<void>&);
    int events() const{return events_;};
    void set_revents(int revents){ revents_ = revents;};
    //判断fd是否有感兴趣的事件


    void update();
    //设置fd响应的事件的状态
    //做按位或运算 如果相应的两个二进制位都为0，则该位的结果为0，否则为1
    void enableReading(){
        events_ |= kReadEvent;
        Logger::LOG_INFO("Channel.h  enableReading  events_ == %d ",events_);
        Logger::LOG_INFO("Channel.h  do update");

        update();

        };
    //先对kreadevent按位取反 对操作数的每个二进制位取反，即0变成1，1变成0 然后呢按位做与运算  都是1的位为1  其余为0
    void disableReading(){events_ &= ~kReadEvent;update();};
    //设置可写入数据
    void enablewriting(){events_ |= KwriteEvent;update();};
    //设置不可写入数据
    void disablewriting(){events_ &= ~KwriteEvent;update();};

    void disableAll(){events_ = kNoneEvent;update();};


    //one loop per thread 判断 当前channel 属于哪个eventloop
    EventLoop *ownerLoop(){return loop_;};

    void remove();


};
