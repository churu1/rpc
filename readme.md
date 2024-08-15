CentOS 8

protobuf 依赖库

前提是得有 GNU 工具：`gcc 和 make`

```shell
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz
tar -xzvf protobuf-cpp-3.19.4.tar.gz
cd protobuf-cpp-3.19.4
./configure -prefix=/usr/local
make -j4
sudo make install
```

tinyxml 依赖库

```shell
wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip
unzip tinyxml_2_6_2.zip

修改 makefile:
OUTPUT := libtinyxml.a

sudo cp libtinyxml.a /usr/lib/
make -j4
sudo mkdir /usr/include/tinyxml
sudo cp tinyxml/*.h /usr/i


up视频或者Readme中的教程最好改一下昂 只改OUTPUT不太行 将其中的OUTPUT := xmltest一行修改为：OUTPUT := libtinyxml.a 将xmltest.cpp从SRCS：=tinyxml.cpp tinyxml-parser.cpp xmltest.cpp tinyxmlerror.cpp tinystr.cpp中删除 注释掉xmltest.o：tinyxml.h tinystr.h。因为不需要将演示程序添加到动态库中。 将${LD} -o @{LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS} 修改为：${AR} @{LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS} 然后再make就可以啦

你可真是我今晚的98%救世主，要是最后那个修改打对了就完美了：
将其中的OUTPUT := xmltest一行修改为OUTPUT := libtinyxml.a

　　将xmltest.cpp从SRCS：=tinyxml.cpp tinyxml-parser.cpp xmltest.cpp tinyxmlerror.cpp tinystr.cpp中删除，

　　注释掉xmltest.o：tinyxml.h tinystr.h。

　　将${LD} -o 
@
{LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}修改为：
${AR} $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}。
我特么狂报stdout版本错误编译不成功，我还以为我用cmake改的有问题链接库不正确。
/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/11/../../../../lib/libtinyxml.a: stdout: invalid version 3 (max 0) /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/11/../../../../lib/libtinyxml.a: error adding symbols: bad value
```



学习过程中相关的问题：

1.什么是线程的全局变量`thread_lcoal`

2.`pintf`和`cout`有什么区别，或者说C语言中的输入输出与C++中的输入输出有什么区别？

3.一些时间相关的函数

4.`__FILE`和`__LINE__`是什么

5.` m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);`是什么

## 日志模块

## EventLoop模块

## Tcp模块
### TcpServer
对于类成员函数作为回调函数，一定要对函数名进行取地址操作（为什么）？

服务器半关闭后（即关闭读写后），为什么还能处理客户端发送来的FIN报文？

为什么将connect用的套接字设置为非阻塞后，是去监听该套接字的可写事件; 我明白了，这个 fd 是用于与服务端通信的fd，当我们调用connect去连接服务端的时候，就是往fd缓冲区中写入内容

rpc相关内容学习：http://www.52im.net/thread-2996-1-1.html

什么是`enable_shared_from_this`类？继承它有什么用？
`dynamic_pointer_cast`的作用是什么？

centOS命令：
`ip add`，查看所有网卡信息
卸载网卡：
``` shell
ifconfig virbr0 down
brctl delbr virbr0
nmcli device delete virbr0-nic
systemctl mask libvirtd.servicesystemctl 
disable libvirtd.service
```
protobuf 与 json 的对比, chatgpt 回答：
`https://chatgpt.com/share/aa34e3b2-0ca9-49d3-a4c6-3fbabfce682f`

为什么在这个项目中，需要将对端的整形通过ntohl转化为本地字节序，而在 FocusTV项目中不需要?

去复习一下四种类型转换的使用场景
