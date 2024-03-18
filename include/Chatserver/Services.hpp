#include<unordered_map>
#include<functional>
#include<string>
#include<Timestamp.h>
#include<memory>
#include"TcpConnection.h"
#include"public.hpp"
#include"Myconn.hpp"
#include"User.hpp"
#include"Usermodel.hpp"
//#include"message.pb.h"

//using probuf = LogRequestProto;
using probuf = int;

//protobuf 版本接口
//using messageHandle = std::function<void(const TcpConnectionPtr &conn,probuf &buf,Timestamp time)> ;

//json版本接口
using messageHandle = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp time)> ;
//业务相关的代码

class Services{
   
    public:
    // 登录业务
    Services();
    ~Services();

    //protobuf 版本   登录服务
    // void login(const TcpConnectionPtr &conn,probuf &buf,Timestamp time);

    //json 版本        登录服务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //注册业务
    // void reg(const TcpConnectionPtr &conn,probuf &buf,Timestamp time);

    //注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);

    static Services* instance() ;//用于获取服务接口的单例 因为Services类 中只是一个回调函数和id一一对应的类 没有自己的数据 
    //获取根据id获取消息处理函数
    messageHandle gethandle(int messageid);
    
    private:

    std::unordered_map<int,messageHandle> messagemap;//存储消息id 和要调用的回调函数  根据id执行回调
    Usermodel usermodel_;
    
};