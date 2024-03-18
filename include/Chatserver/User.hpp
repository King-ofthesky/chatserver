#include<string>
#ifndef USER_HPP
#define USER_HPP
class User
{
/*
用于存放读取User字段的类
CREATE TABLE user (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(32) NOT NULL,
    passwd VARCHAR(32) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline'
);

*/
public:
    User(int id = -1,std::string name = "",std::string passwd = "",std::string  state = "offline"){
        this->id = id;
        this->name = name;
        this->passwd = passwd;
        this->state = state;
    }
    ~User(){
        
    }
    

    void setId(int id){
        this->id = id;  
    }

    void setName(std::string name){
        this->name= name;
    }

    void setPasswd(std::string passwd){
        this->passwd= passwd;
    }

    void setState(std::string state){
        this->state= state;
    }

    int getId(){
        return id;
    }

    std::string  getPasswd(){
        return passwd;
    }

    std::string  getName(){
        return name;
    }

    std::string  getState(){
        return state;
    }

private:
    int id;
    std::string name;
    std::string passwd;
    std::string  state;
};

#endif