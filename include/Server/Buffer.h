#pragma once

#include<vector>
#include<string>
#include<algorithm>

#include"noncopyable.h"
#include"Logger.h"
class Buffer
{
public: 
    static const size_t KCheapPrepend_ = 8;   //8字节数据包长度
    static const size_t KInitialSize_ = 1024;   //初始缓冲区大小
    explicit Buffer(size_t initialSize = KInitialSize_)
            :buffer_(std::vector<char>(KCheapPrepend_+KInitialSize_))
            ,readIndex_(KCheapPrepend_)
            ,writeIndex_(KCheapPrepend_)
    {
    }
    //可以读取长度  
    size_t readableBytes()const
    { 
                                           //写指针减去读指针就是写入数据的长度
        return writeIndex_ - readIndex_;  //可以读的数据长度   8byte|  readindex  | writeindex  =  wireindex - readindex
    }
    //可写入长度
    size_t writeableBytes()const
    {
        return buffer_.size() - writeIndex_; // buffer的大小减去写指针就是剩下可以写的大小
    }
    //当前读索引
    size_t prependableBytes()const
    {
        return readIndex_;
    }
    
    //读指针
    const  char*  peek()const
    {
        return begin() + readIndex_;
    }

     // 对缓冲区复位
    void retrieve(size_t len)
    {  
        if(len < readableBytes())//读了len 没读完  
        {
            readIndex_ +=len;//下次继续读
        }else//读取完  复位
        {
            retrieveAll();
        }
    }
        //全部复位 
        void retrieveAll()
    {
        readIndex_= writeIndex_ =KCheapPrepend_;//全部复位 
    }
     
     // 返回一个string 对象
    std::string retrieveAllAsstring()
    {
        return retrieveAsstring(readableBytes());
    }

    // 创建一个栈上的string对象  复制完数据后复位  返回的是一个栈上的对象
    std::string retrieveAsstring(size_t len)
    {
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }

    //写数据 前判断是否扩容 提前扩容防止多次拷贝
    void ensureWriteableIndex(size_t len)
    {

        if(writeableBytes()<len)
        {
            makeSpace(len);//扩容
        }

    }
    
    //将len字节data数据写入 write缓冲区
    void append(const char* data,size_t len)
    {
        //写数据前先判缓冲区是否需要扩容
        
        ensureWriteableIndex(len);
        //从data位置 复制len个数据到beginwritte位置 
        std::copy(data,data+len,beginwrite());//写入数据 向beginwrite这地址 写入地址为data到data+len 的数据
        writeIndex_+=len;//写入数据后更新write指针
    }

    //获取写索引位置指针
    char* beginwrite()
    {
        return begin()+ writeIndex_;//数组首指针 加写索引偏移量 得到 执行写索引位置的指针
    }

    const char* beginwrite()const
    {
        return begin()+ writeIndex_;//数组首指针 加写索引偏移量 得到 执行写索引位置的指针
    }

    void haswriten()
    {

    }

    //从fd上读取数据
    ssize_t readfd(int fd,int* rerrno);
    //从fd上发送数据
    ssize_t writefd(int fd,int* rerrno);
private:
    void makeSpace(size_t len)//扩容
    {
        if(writeableBytes()+prependableBytes()< len + KCheapPrepend_)//写的下
        {
            buffer_.resize(writeIndex_+len);
        }else
        {
            size_t readable = readableBytes();//读的长度
            //std::copy(source.begin(), source.end(), destination.begin());  
            //把读的数据拷贝到
            std::copy(begin()+readIndex_,
                    begin() + writeIndex_,
                    begin() + KCheapPrepend_
                    );
                
            readable = KCheapPrepend_;
            writeIndex_ = readIndex_ + readable;
        }
    }

    //返回buffer_内部第一个元素的地址 
    char* begin()
    {
        // return &(*(buffer_.begin()));   //获取迭代器  *运算 得到第一个元素  对第一个元素解引用得到第一个元素地址
        return buffer_.data();
    }

    const char* begin()const
    {
        // return &(*(buffer_.begin()));   //获取迭代器  *运算 得到第一个元素  对第一个元素解引用得到第一个元素地址
        return buffer_.data();
    }
    // buffer ->>      8字节|read|write   ;



    std::vector<char> buffer_;//buff

    size_t readIndex_;//读索引
    size_t writeIndex_;//写索引
    static const char KCLRF[];  //换行符序列的静态常量字符数组
};