#pragma once

#include"Mainserver.hpp"
#include"TcpServer.h"
#include"EventLoop.h"
int main()
{
    EventLoop loop;
    
    InetAddress addr(6666, "192.168.137.3");

    Mainserver server(&loop, addr, "server");

    server.statr();

    loop.loop();

    return 0;

}