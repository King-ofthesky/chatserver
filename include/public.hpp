#include"json.hpp"
using json = nlohmann::json;

#ifndef PUBLIC_H
#define PUBLIC_H

enum msgmaptype{
    LOGIN_MSG = 1,
    LOGIN_MSG_ACK = 2,
    REG_MSG = 3,
    REG_MSG_ACK = 4
};


#endif
//此头文件是客户端和服务端消息类型的定义 