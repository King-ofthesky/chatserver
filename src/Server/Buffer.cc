#include"Buffer.h"
#include<unistd.h>
#include<errno.h>
#include<sys/uio.h>
//从fd上读取数据 
/*
LT模式 
    1"水平触发：在 LT 模式下，只要文件描述符上有数据可读或可写，内核就会不断通知应用程序。
这意味着如果应用程序没有处理完所有数据，下次循环时仍会收到通知。
    2非阻塞 I/O：对于非阻塞的文件描述符，即使缓冲区已满或已空，仍然会触发可读或可写事件
    3应用程序只需在每次循环中读取或写入尽可能多的数据即可

*/
const char KCLRF[] = "\r\n";



ssize_t Buffer::readfd(int fd,int* rerrno)
{  
    //
    // Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据到缓冲区 writeableBytes == %d readableBytes == %d", writeableBytes(),readableBytes());
    char extrabuff[65536] = {0};//栈上内存  64kb
    struct iovec  vec[2];  //多段缓冲区  内部是一个指针和一个size_t len
    //可写入长度
    const size_t writeable = writeableBytes();

    vec[0].iov_base = begin()+ writeIndex_;  //
    vec[0].iov_len =  writeable;

    vec[1].iov_base = extrabuff;
    vec[1].iov_len = sizeof(extrabuff); 
    
    const int iovcont = (writeable <sizeof(extrabuff)) ? 2 : 1;
    // const int iovcont = 2;
    // Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据 可写入长度为  size  == %d  可读长度为 ==  %d", writeableBytes(),readableBytes());  
    // Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据 readindex  == %d  writeindex  ==  %d", readIndex_,writeIndex_);   
    const ssize_t readlen = ::readv(fd,vec,iovcont);//从fd中读数据到vec   

    if(readlen < 0)//读取失败
    {
        Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据 读取失败");
        *rerrno = errno;//读取失败的错误  
    }
    else if(readlen <= writeable)//读取数据长度 小于可写入长度  说明只读取到buff中
    {

        writeIndex_ += readlen;// 更新写入数据的指针   写入数据的指针指向读取的数据末尾
        // std::string buff(buffer_.data()+readIndex_,readableBytes());
        Logger::LOG_INFO("Buffer.cc.cc readfd   readv readlen == %d ", readlen);  
        // Logger::LOG_INFO("Buffer.cc.cc readfd   readv 可写入长度为  size  == %d  可读长度为 ==  %d", writeableBytes(),readableBytes());  
        // Logger::LOG_INFO("Buffer.cc.cc readfd   readv  readindex  == %d  writeindex  ==  %d", readIndex_,writeIndex_);   
        Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据 刚好够");
        // Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据为  %s",buff.c_str());
    }
    else  //buffer读满了  向第二个栈存数据
    {

        writeIndex_ = buffer_.size();
        // Logger::LOG_INFO("Buffer.cc.cc readfd 从fd中读数据 扩容 执行append size = %d readlen =  %d ",readlen - writeable,readlen);
        append(extrabuff,readlen - writeable); // 扩容buffer 将第二段缓冲内容读到buffer中
    }
    return readlen;
}


ssize_t Buffer::writefd(int fd,int* rerrno){
    //向缓冲器 读指针 读readableBytes长度数据
    ssize_t n = ::write(fd,peek(),readableBytes());
    if(n < 0){  //读取出错
        *rerrno = errno;
    }
    return n;

}