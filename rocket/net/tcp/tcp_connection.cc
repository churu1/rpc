#include <unistd.h>
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/log.h"
#include "tcp_connection.h"

namespace rocket {



TcpConnection::TcpConnection(EventLoop* event_loop, int conn_fd, int buffer_size, NetAddr::s_ptr peer_addr) 
  :m_event_loop(event_loop), m_fd(conn_fd), m_peer_addr(peer_addr), m_state(NotConnected){

  m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
  m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

  m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(conn_fd);
  m_fd_event->setNonBlock();
  m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::OnRead, this));
  m_event_loop->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection() {
  DEBUGLOG("~TcpConnection");
}

void TcpConnection::OnRead() {
  // 1.从socket缓冲区，调用系统的 read 函数读取字节到 in_buffer 里面
  if (m_state != Connected) {
    ERRORLOG("OnRead error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
    return;
  }

  bool is_read_all = false;
  bool is_close = false;
  while (!is_read_all) {
    if (m_in_buffer->writeAble() == 0) {
      m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
    }
    int read_count = m_in_buffer->writeAble();
    int write_index = m_in_buffer->writeIndex();

    int rt = read(m_fd_event->getFd(), &(m_in_buffer->m_buffer[write_index]), read_count);
    DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
    if (rt > 0) {
      m_in_buffer->moveWriteIndex(rt);

      if (rt == read_count) {
        continue;
      } else if (rt < read_count) {
        is_read_all = true;
        break;
      }

    } else if (rt == 0) {
      is_close = true;
      break;
    } else if (rt == -1 && errno == EAGAIN) {
      is_read_all = true;
      break;
    }
  }

  if (is_close) {
    //TODO 处理关闭连接
    INFOLOG("peer closed, peer addr[%s], client fd[%d]", m_peer_addr->toString().c_str(), m_fd)
    clear();
    return;
  }
  if (!is_read_all) {
    ERRORLOG("not read all data");
  }

  // TODO 简单的 echo, 后面补充 RPC 协议的计息
  excute();
}


void TcpConnection::excute() {
  DEBUGLOG("start excute");
  // 将 RPC 请求执行业务逻辑，获取 RPC 响应，再把 RPC 相应发送回去
  std::vector<char> tmp;
  int size = m_in_buffer->readAble();
  m_in_buffer->readFromBuffer(tmp, size);


  std::string msg;
  for (int i = 0; i < (int)tmp.size(); ++i) {
    msg += tmp[i];
  }
  
  INFOLOG("success get request[%s] from peer client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

  m_out_buffer->writeToBuffer(msg.c_str(), msg.length());

  m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::OnWrite, this));

  m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::OnWrite() {
  // 将当前 out_buffer 里面的数据全部发送给 client

  if (m_state != Connected) {
    ERRORLOG("OnWrite error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd );
    return;
  }

  bool is_write_all = false;
  while (true) {
    if (m_out_buffer->readAble() == 0) {
      DEBUGLOG("no data need to send to client[%s]", m_peer_addr->toString().c_str());
      is_write_all = true;
      break;
    }
    int write_size = m_out_buffer->readAble();
    int read_index = m_out_buffer->readIndex();

    int rt = write(m_fd, &m_out_buffer->m_buffer[read_index], write_size);
    DEBUGLOG("success write %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);

    if (rt >= write_size) {
      DEBUGLOG("no data need to send to client[%s]", m_peer_addr->toString().c_str());
      is_write_all = true;
      break;
    }
    
    if (rt == -1 && errno == EAGAIN) {
      // 发送缓冲区已满，不能再发送
      // 这种请开给你直接等下次 fd 可写再发送
      ERRORLOG("write data error, errno==EAGAIN and rt == -1");
      break;
    }
  }
  
  if (is_write_all) {
    m_fd_event->cancel(FdEvent::OUT_EVENT);
    m_event_loop->addEpollEvent(m_fd_event);
  }

}

void TcpConnection::setState(const TcpState) {
  m_state = Connected;

}

TcpState TcpConnection::getState() {
  return m_state;
}

void TcpConnection::clear() {
  // 处理一些关闭连接后的清理动作

  if (m_state == Closed) {
    return;
  }

  m_fd_event->cancel(FdEvent::IN_EVENT);
  m_fd_event->cancel(FdEvent::OUT_EVENT);


  m_event_loop->deleteEpollEvent(m_fd_event);

  m_state = Closed;


}

// 针对大量客户端连接成功，但是这些连接没有进行数据交换，从而浪费服务器资源
void TcpConnection::shutdown() {
  if (m_state == Closed || m_state == NotConnected) {
    return;
  }

  // 处于半关闭状态
  m_state = HalfClosing;

  // 调用 shutdown 关闭读写， 意味着服务器不会再对这个 fd 进行读写操作
  // 发送 FIN 报文，处罚了四次挥手的第一个阶段
  // 当 fd 发生可读事件，但是可读数据为 0，即对端发送了 FIN
  ::shutdown(m_fd, SHUT_RDWR);
}

void TcpConnection::setTcpConnectionType(TcpConnectionType type) {
  m_connection_type = type;
}
}
