#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <memory>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/timer_event.h"
#include "rocket/net/io_thread_group.h"

void test_io_thread() {

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    ERRORLOG("listenfd = -1");
    exit(0);
  }

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_port = htons(12346);
  addr.sin_family = AF_INET;
  inet_aton("127.0.0.1", &addr.sin_addr);

  int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  if (rt != 0) {
    ERRORLOG("bind error");
    exit(0);
  }
  DEBUGLOG("bind success");

  rt = listen(listenfd, 100);
  if (rt != 0) {
    ERRORLOG("listen error");
    exit(0);
  }
  DEBUGLOG("listen success");

  rocket::FdEvent event(listenfd);
  event.listen(rocket::FdEvent::IN_EVENT, [listenfd]() {
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);
    int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    DEBUGLOG("success get client fd[%d] client info[%s:%d]", clientfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  });

  int i = 0;
  rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(1000, true, [&i](){
    INFOLOG("trigger timer event, count=%d", i++);
  });

  rocket::IOThreadGroup io_thread_group(2);
  rocket::IOThread* io_thread = io_thread_group.getIOThread();
  io_thread->getEventLoop()->addEpollEvent(&event);
  io_thread->getEventLoop()->addTimerEvent(timer_event);


  rocket::IOThread* io_thread2 = io_thread_group.getIOThread();
  io_thread2->getEventLoop()->addTimerEvent(timer_event);

  io_thread_group.start();
  io_thread_group.join();


  // rocket::IOThread io_thread;

  // io_thread.getEventLoop()->addEpollEvent(&event);
  // io_thread.getEventLoop()->addTimerEvent(timer_event);

  // io_thread.start();
  // io_thread.join();
}

int main() {

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");
  rocket::Logger::InitGlobalLogger();

  test_io_thread();

  // rocket::EventLoop* eventloop = new rocket::EventLoop();

  // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  // if (listenfd == -1) {
  //   ERRORLOG("listenfd = -1");
  //   exit(0);
  // }

  // sockaddr_in addr;
  // memset(&addr, 0, sizeof(addr));

  // addr.sin_port = htons(12345);
  // addr.sin_family = AF_INET;
  // inet_aton("127.0.0.1", &addr.sin_addr);

  // int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  // if (rt != 0) {
  //   ERRORLOG("bind error");
  //   exit(0);
  // }
  // DEBUGLOG("bind success");

  // rt = listen(listenfd, 100);
  // if (rt != 0) {
  //   ERRORLOG("listen error");
  //   exit(0);
  // }
  // DEBUGLOG("listen success");

  // rocket::FdEvent event(listenfd);
  // event.listen(rocket::FdEvent::IN_EVENT, [listenfd]() {
  //   sockaddr_in client_addr;
  //   memset(&client_addr, 0, sizeof(client_addr));
  //   socklen_t client_len = sizeof(client_addr);
  //   int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
  //   DEBUGLOG("success get client fd[%d] client info[%s:%d]", clientfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  // });

  // int i = 0;
  // rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(1000, true, [&i](){
  //   INFOLOG("trigger timer event, count=%d", i++);
  // });

  // rocket::IOThreadGroup io_thread_group(2);
  // rocket::IOThread* io_thread = io_thread_group.getIOThread();
  // io_thread->getEventLoop()->addEpollEvent(&event);
  // io_thread->getEventLoop()->addTimerEvent(timer_event);


  // rocket::IOThread* io_thread2 = io_thread_group.getIOThread();
  // io_thread2->getEventLoop()->addTimerEvent(timer_event);

  // io_thread_group.start();
  // io_thread_group.join();



  // eventloop->addTimerEvent(timer_event);
  // eventloop->loop();
  
  return 0;
}