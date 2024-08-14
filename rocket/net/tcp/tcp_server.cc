#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/common/log.h"
#include "tcp_server.h"
namespace rocket {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {

  init();

  INFOLOG("rocket RPC TcpServer listen success on [%s]", m_local_addr->toString().c_str());
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
  auto re = m_acceptor->accpet();
  int client_fd = re.first;
  NetAddr::s_ptr  peer_addr = re.second;
  m_client_counts++;

  IOThread* io_thread = m_io_thread_group->getIOThread();
  TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr);
  connection->setState(Connected);
  m_clients.insert(connection);
  INFOLOG("TcpServer succ get client, fd=%d", client_fd);
}

}