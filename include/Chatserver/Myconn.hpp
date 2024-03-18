#pragma once
#include<mysql/mysql.h>
#include<string>
static std::string dbuser = "root";         //数据库用户名
static std::string dbpasswd = "root";       //数据库密码
static std::string dbhost  = "127.0.0.1"; //数据库ip地址
static std::string dbport = "3306" ;                  //数据库端口
static std::string databasename = "myserver1"; //使用的数据库的名字

class Myconn
{

public:
    Myconn();
    ~Myconn();

    bool connect();//连接数据库

    bool update(std::string sql);//非查询操作

    MYSQL_RES* query(std::string sql);//查询操作

    MYSQL* getMysqlConnection();



private:

    MYSQL *conn_;//官方 c语言 连接器

};
