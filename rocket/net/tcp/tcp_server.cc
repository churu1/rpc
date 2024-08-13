#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "tcp_server.h"
namespace rocket {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {

  init();

  INFOLOG("rocket RPC TcpServer listen success on [%s]", m_local_addr->toString());
}

TcpServer::~TcpServer() {
  if (m_main_event_loop) {
    delete m_main_event_loop;
    m_main_event_loop = NULL;
  }

  if (m_io_thread_group) {
    delete m_io_thread_group;
    m_io_thread_group = NULL;
  }

  if (m_listen_fd_event) {
    delete m_listen_fd_event;
    m_listen_fd_event = NULL;
  }
}

void TcpServer::start() {
  m_io_thread_group->start();
  m_main_event_loop->loop();

}

void TcpServer::init() {
  m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

  m_main_event_loop = EventLoop::GetCurrentEventLoop();

  m_io_thread_group = new IOThreadGroup(2);

  m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());

  m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));
  m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
  int client_fd = m_acceptor->accpet();
  //FdEvent client_fd_event(client_fd);
  m_client_counts++;

  //TODO 把 client_fd 添加到任意的 io 线程里面
  //m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent()

  INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

}