#include"User.hpp"
#include"Myconn.hpp"
#ifndef USERMODEL_HPP
#define USERMODEL_HPP

class Usermodel
{
private:
    public:
    Usermodel();
    ~Usermodel();
    bool insert(User &user);//用于增加向user表添加user
    bool delete1(User &user);
    private:

};

#endif