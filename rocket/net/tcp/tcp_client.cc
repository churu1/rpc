#include <sys/socket.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/common/log.h"
#include "rocket/net/fd_event_group.h"


namespace rocket {
TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
  m_event_loop = EventLoop::GetCurrentEventLoop();
  m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
  if (m_fd < 0) {
    ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
    return;
  }

  m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);

  m_connection = std::make_shared<TcpConnection>();
}

TcpClient::~TcpClient()
{
}
void TcpClient::connect(std::function<void()> done)
{
}
void TcpClient::writeMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done)
{
}
void TcpClient::readMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done)
{
}
}
