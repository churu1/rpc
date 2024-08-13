#include <sys/timerfd.h> // ?
#include <string.h>
#include "rocket/net/timer.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "timer.h"


namespace rocket {




Timer::Timer() : FdEvent(){

  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

  DEBUGLOG("timer fd=%d", m_fd);

  // 把 fd 的可读事件放到 eventloop 上监听
  listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {

}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
  bool is_reset_timerfd = false;

  ScopeMutex<Mutex> locker(m_mutex);
  if (m_pending_events.empty()) {
    is_reset_timerfd = true;
  } else {
    auto it = m_pending_events.begin();
    if ((*it).second->getArriveTime() > event->getArriveTime()) {
      is_reset_timerfd = true;
    }
  }
  m_pending_events.emplace(event->getArriveTime(), event);
  locker.unlock();
  if (is_reset_timerfd) {
    resetArriveTime();
  }
  DEBUGLOG("success add tiemr evnet");
}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
  event->setCancler(true);

  ScopeMutex<Mutex> locker(m_mutex);
  auto begin = m_pending_events.lower_bound(event->getArriveTime());
  auto end = m_pending_events.upper_bound(event->getArriveTime());

  auto it = begin;
  for (auto it = begin; it != end; ++it) {
    if (it->second == event) {
      break;
    }
  }

  if (it != end) {
    m_pending_events.erase(it);
  }
  locker.unlock();
  DEBUGLOG("success delete TimerEvent at arrive time %lld", event->getArriveTime());
}


void Timer::onTimer() {

  DEBUGLOG("on timer");
  
  // 处理缓冲区数据，防止继续触发可读事件
  // 注意这里是LT模式下的非阻塞读， 不是ET模式下的非阻塞读
  char buf[8];
  while (1) {
    if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
      break;
    }
  }

  // 执行定时任务
  int64_t now = getNowMs();

  std::vector<TimerEvent::s_ptr> tmps;
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;

  ScopeMutex<Mutex> locker(m_mutex);
  auto it = m_pending_events.begin();
  for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
    if (it->first <= now) {
      if (!(it->second->getIsCancled())) {
        tmps.push_back(it->second);
        tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
      }
    } else {
      break;
    }
  }

  m_pending_events.erase(m_pending_events.begin(), it);
  locker.unlock();

  // 需要把重复的 Event 再次添加
  for (auto it = tmps.begin(); it != tmps.end(); ++it) {
    if ((*it)->getIsRepated()) {
      // 调整 arriveTime
      (*it)->resetArriveTime();
      addTimerEvent(*it);
    }
  }

  resetArriveTime();
  for (auto it : tasks) {
    if (it.second) {
      it.second();
    }
  }
}

void Timer::resetArriveTime() {
  ScopeMutex<Mutex> locker(m_mutex);
  auto tmp = m_pending_events;
  locker.unlock();

  if (tmp.size() == 0) {
    return;
  }

  int64_t now = getNowMs();

  auto it = tmp.begin();
  int64_t inteval = 0;
  if (it->second->getArriveTime() > now) {
    inteval = it->second->getArriveTime() - now;
  } else {
    inteval = 100;
  }
  DEBUGLOG("inteval %lld", inteval);

  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = inteval / 1000;
  ts.tv_nsec = (inteval % 1000) * 1e6;

  itimerspec value;
  memset(&value, 0, sizeof(value));
  value.it_value = ts;

  int rt = timerfd_settime(m_fd, 0, &value, NULL);
  if (rt != 0) {
    ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
  }
  DEBUGLOG("tiemr reset to %lld", (now + inteval));
}














}
