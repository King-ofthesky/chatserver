#pragma once
#include<iostream>
#include"Myconn.hpp"
Myconn::Myconn()
{
    conn_ = mysql_init(nullptr);  //默认初始化MYSQL结构体
}

Myconn::~Myconn()
{
    if(conn_ != nullptr)
    {
        mysql_close(conn_); //关闭
    }
}

bool Myconn::connect()//连接mysql操作
{
    /**
     * mysql：一个指向已初始化的 MYSQL 结构体的指针，用于存储数据库连接的状态和信息。
    host：要连接的 MySQL 服务器主机名或 IP 地址。
    user：连接 MySQL 数据库的用户名。
    passwd：连接 MySQL 数据库的密码。
    db：要选择的初始数据库或 schema 名称。
    port：MySQL 服务器的端口号。
    unix_socket：UNIX 套接字路径（通常用于本地连接，可以为 nullptr）。
    client_flag：客户端标志，用于指定连接属性和选项。 
    */
    MYSQL *M = mysql_real_connect(conn_, dbhost.c_str(), dbuser.c_str(),
                        dbpasswd.c_str(), databasename.c_str(), std::stoi(dbport), 
                        nullptr, 0);
    if(M == nullptr)
    {
        std::cout<<__FILE__ << "::"<<__LINE__<<"  connect 数据库 链接 失败 "<<std::endl;

    }
    return M;
}   

//执行sql语句
bool Myconn::update(std::string sql)//用来执行非查询操作
{
    if(mysql_query(conn_,sql.c_str()))  //使用conn_ 执行 sql 
    {
        std::cout<<__FILE__ << "::"<<__LINE__<<"  connect sql执行 失败 "<<sql.c_str()<<std::endl;
        return false;
    }
    return true;//sql执行成功
}


MYSQL_RES* Myconn::query(std::string sql)   //用来执行查询操作
{
    if(mysql_query(conn_,sql.c_str()))//使用conn_ 执行 sql   
    {
        std::cout<<"Myconn.cpp   query   sql执行失败"<<std::endl;
        return nullptr; 
    }
    return mysql_use_result(conn_);//只能获得上一次查询结果  
}


MYSQL* Myconn::getMysqlConnection()
{
    return conn_;
}