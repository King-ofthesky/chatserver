#include"Services.hpp"

//构造函数 注册回调操作
Services::Services()
{
    // std::unordered_map<int,messageHandle> messagemap;
    //通过绑定器绑定回调函数  服务器根据这个表 的id来执行回调函数操作

    messagemap.insert({LOGIN_MSG,std::bind(&Services::login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    messagemap.insert({REG_MSG,std::bind(&Services::reg,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    
}

// void Services::login(const TcpConnectionPtr &conn,probuf &buf,Timestamp time)
// {
    
// }

// void Services::reg(const TcpConnectionPtr &conn,probuf &buf,Timestamp time)
// {
    
// }

void Services::login(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    Logger::LOG_INFO("do login");


}

void Services::reg(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    //从json中读取数据  
    Logger::LOG_INFO("do reg");
    std::string name = js["name"];
    std::string pwd = js["passwd"];
    User user;  // 构造 数据对象
    user.setName(name);
    user.setPasswd(pwd);
    bool state = usermodel_.insert(user);  //通过 UserModel 执行insert语句 插入一个User对象 并获取是否执行成功 会自动设置id
    if(state)
    {  
        //注册成功
        json message;
        message["messageid"] = REG_MSG_ACK;   //客户端调用的回调函数
        message["errno"] = 0;//注册成功
        message["userid"] = user.getId();
        conn->send(message.dump()); //发送数据

    }else{
        json message;
        message["messageid"] = REG_MSG_ACK;   //客户端调用的回调函数
        message["errno"] = 1;  //注册失败
        conn->send(message.dump()); //发送数据
    }
    
}

messageHandle Services::gethandle(int messageid)
{
    //记录错误日志  id没有对应的事件处理回调  黑客攻击 封他号
    auto it =messagemap.find(messageid);
    if(it == messagemap.end()){   //没有找到这个id
        Logger::LOG_INFO("");
        // return [=](const TcpConnectionPtr &conn,probuf &buf,Timestamp time){
        //     std::string err = "id  " + std::to_string(messageid) + "  not fuound";
        // };
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp time){
            std::string err = "id  " + std::to_string(messageid) + "  not fuound";
        };
    }else{
        return messagemap[messageid];
    }
    
}

//静态方法  无需调用对象的指针
Services* Services::instance()
{
    static Services services;
    return &services;
}

Services::~Services()
{

}