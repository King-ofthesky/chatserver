one loop per thread 模型
 
Eventloopthread 是对线程封装的抽象  创建这个对象 等价与创建一个新线程 和eventloop

EventloopthreadPoll 是线程池 管理Eventloopthread, 可以根据cpu核心数量自动创建线程池


一个eventloop中有一个poller  和多个channel

一个poller中监听多个channel、

cannel中封装了 sockfd   和 感兴趣的 event, 包括 EPOLLIN  EPOLLOUT  EPOLLERR事件

对外放编程的类包括TcpServer TcpConnection

服务器源码在 project/src/server/ project/include/server/下 
可以直接复制这2个文件夹内的代码直接用
也可以自己去编译成库文件 

其他文件夹的代码和Tcp服务器源码无关  是业务模块

日志信息是自己随意编写的 不想看这个日志可以去project/src/server/Logger.cc 下修改
int Logger::logminlevel_ 的值 大于4  这样就没日志了
或者自己去对应的代码位置修改日志内容


目前已经测试完成 可以正常建立连接 接受数据 和发送数据