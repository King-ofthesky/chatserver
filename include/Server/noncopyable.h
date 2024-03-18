#pragma once

//继承了noncopyable的类 无法拷贝 和赋值  因为 派生类拷贝和赋值时总要先调用父类的拷贝和赋值
class noncopyable{
    public: 
    //删除拷贝构造函数
    noncopyable(const noncopyable&) = delete;
    //删除赋值构造函数
    void  operator=(const noncopyable&) = delete;
    private:

    //构造函数和析构函数只有基类可以调用
    protected:
    //构造函数和析构函数默认实现
    noncopyable() = default;
    ~noncopyable() = default;

};