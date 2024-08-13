CentOS 8

protobuf 依赖库

前提是得有 GNU 工具：`gcc 和 make`

```shell
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz
tar -xzfv protobuf-cpp-3.19.4.tar.gz
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

sudp cp libtinyxml.a /usr/lib/
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

