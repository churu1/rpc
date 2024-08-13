#include <memory>
#include <string.h>
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/common/log.h"


namespace rocket {
TcpBuffer::TcpBuffer(int size) {
  m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {
}

int TcpBuffer::readAble() {
  return m_write_idx - m_read_idx;
}

int TcpBuffer::writeAble() {
  return m_buffer.size() - m_write_idx;
}


int TcpBuffer::readIndex() {
  return m_read_idx;
}


int TcpBuffer::writeIndex() {
  return m_write_idx;
}


void TcpBuffer::writeToBuffer(const char *buf, int size) {
  if (size > writeAble()) {
    // 调整 buffer 大小， 扩容
    int new_size = (int)(1.5 * (m_read_idx + size));
    resizeBuffer(new_size);
  } 
  memcpy(&m_buffer[m_write_idx], buf, size);
  m_write_idx += size;
}


void TcpBuffer::readFromBuffer(std::vector<char> &re, int size) {
  if (readAble() == 0) return;

  int read_size = readAble() > size ? size : readAble();
  DEBUGLOG("read_size [%d], size[%d]", read_size, size);
  std::vector<char> tmp(read_size);
  memcpy(&tmp[0], &m_buffer[m_read_idx], read_size);
  re.swap(tmp);

  m_read_idx += read_size;

  adjustBuffer();
}

void TcpBuffer::resizeBuffer(int new_size) {
  std::vector<char> tmp(new_size);
  int count = std::min(new_size, readAble());
  memcpy(&tmp[0], &m_buffer[m_read_idx], count);
  m_buffer.swap(tmp);

  m_read_idx = 0;
  m_write_idx = m_read_idx + count;
}

void TcpBuffer::adjustBuffer() {
  if (m_read_idx < int(m_buffer.size() / 3))  return;

  std::vector<char> buffer(m_buffer.size());
  int count = readAble();

  memcpy(&buffer[0], &m_buffer[m_read_idx], count);
  m_buffer.swap(buffer);
  m_read_idx = 0;
  m_write_idx = m_read_idx + count;

  buffer.clear();
}

void TcpBuffer::moveReadIndex(int size) {
  size_t j = m_read_idx + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_idx, m_buffer.size());
    return;
  }
  m_read_idx = j;
  adjustBuffer();
}


void TcpBuffer::moveWriteIndex(int size) {
  size_t j = m_write_idx + size;
  if (j >= m_buffer.size()) {
    ERRORLOG("moveWriteIndex error, invalid size %d, old_write_index %d, buffer size %d", size, m_write_idx, m_buffer.size());
    return;
  }
  m_write_idx = j;
  adjustBuffer();
}


}