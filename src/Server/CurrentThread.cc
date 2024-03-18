#include"CurrentThread.h"
namespace currentthread{
    __thread int t_cacheTid = 0;

    void cacheTid(){    
        if(t_cacheTid == 0){
            //通过linux系统调用获得当前线程tid值
            t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid)); 
        }
    }
    
}