#pragma once 
#include<unistd.h>
#include<syscall.h>


namespace currentthread{

    //线程全局变量  每个线程独有
    /*
    这段代码使用了C语言的extern关键字和GCC扩展的__thread关键字来声明一个线程局部存储的整型变量t_cacheTid。
    extern 关键字用于声明一个外部变量，表示该变量在其他地方定义或实现，当前位置仅为其声明。在这段代码中，它告诉编译器t_cacheTid变量的定义在
    其他地方。
    __thread 是GCC扩展的关键字，用于声明线程局部存储变量。它指示编译器为每个线程分配独立的存储空间，每个线程都有自己的t_cacheTid变量副本，
    互不干扰。因此，这句话的意思是声明了一个名为t_cacheTid的整型变量，在每个线程中都有自己独立的副本，并且该变量的定义在其他地方。
    需要注意的是，具体的定义和初始化可能在其他文件中进行，而这里只是声明该变量的存在。
    */
   extern  __thread int t_cacheTid ;
    
    void cacheTid();

/*

这段代码使用了内建函数__builtin_expect()，它是一种在编译器层面优化程序性能的机制。

__builtin_expect()函数用于提示编译器一个分支语句的概率，以便编译器可以进行相应的优化。其语法为：__builtin_expect(expr, value)


expr 是一个表达式，通常是一个条件判断语句。
value 是expr的预期结果，通常是0或1，表示该条件的概率。
在给定的示例代码中，__builtin_expect(t_cacheTid==0, 0)表示条件t_cacheTid==0的预期结果为0，即条件成立的概率较低。

接下来的代码块是一个条件语句。如果条件t_cacheTid==0的预期结果为0，则执行以下代码：
*/

    inline int tid(){
        if(__builtin_expect(t_cacheTid==0,0)){
            void cacheTid();
        }
        return t_cacheTid;
    }



}