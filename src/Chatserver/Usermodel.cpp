#include<string>
#include<iostream>
#include"Usermodel.hpp"



bool Usermodel::insert(User &user)
{
    //INSERT INTO user (name, passwd, state) VALUES ('John', 'password123', 'online');
    char sql[1024] = {0};
    snprintf(sql,1024,
    "insect into user(name, passwd, state) VALUES  ('%s', '%s', '%s')"
    ,user.getName().c_str(),user.getPasswd().c_str()
    );
    Myconn conn;
    if(conn.update(sql)){
        user.setId(mysql_insert_id(conn.getMysqlConnection()));//获取上一个自增的id 也就是刚插入的id
        return true;
    }
    
    return false;

}

Usermodel::Usermodel()
{

}

Usermodel::~Usermodel()
{
    
}