#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"
#include "rocket/common/run_time.h"

namespace rocket {
Logger* g_logger = NULL;

Logger* Logger::GetGlobalLogger() {

  return g_logger;
}

void Logger::InitGlobalLogger() {
  LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
  g_logger = new Logger(global_log_level);
  g_logger->init();
}

std::string logLevelToString(LogLevel level) {
  switch (level) {
  case Debug:
    return "DEBUG";
    break;
  case Info:
    return "INFO";
    break;
  case Error:
    return "ERROR";
    break;
  default:
    return "UNKNOWN";
    break;
  }

}

LogLevel StringToLogLevel(const std::string log_level) {
  if (log_level == "DEBUG") {
    return Debug;
  } else if (log_level == "INFO") {
    return Info;
  } else if (log_level == "ERROR") {
    return Error;
  } else {
    return Unknown;
  }
}


std::string LogEvent::toString() {
  struct timeval now_time;

  gettimeofday(&now_time, nullptr);
  
  struct tm now_time_t;
  localtime_r(&(now_time.tv_sec), &now_time_t);

  char buf[128];
  strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
  std::string time_str(buf);

  int ms = now_time.tv_usec / 1000;
  time_str = time_str + ":" + std::to_string(ms);

  m_pid = getPid();
  m_thread_id = getThreadId();

  std::stringstream ss;
  ss << "[" << logLevelToString(m_level) << "]\t"
    << "[" << time_str << "]\t"
    << "[" << m_pid << ":" << m_thread_id << "]\t";

  
  // 获取当前线程处理的请求的 msgid

  std::string msgid = RunTime::GetRunTime()->m_msgid;
  std::string method_name = RunTime::GetRunTime()->m_method_name;

  if (!msgid.empty()) {
    ss << "[" << msgid << "]\t";
  }
  
  if (!method_name.empty()) {
    ss << "[" << method_name << "]\t";
  }

  return ss.str();
}

Logger::Logger(LogLevel level) : m_set_level(level) {
  m_async_logger = std::make_shared<AsyncLogger>(Config::GetGlobalConfig()->m_log_file_name + "_rpc",
  Config::GetGlobalConfig()->m_log_file_path,
  Config::GetGlobalConfig()->m_log_max_file_size);
  
  m_async_app_logger = std::make_shared<AsyncLogger>(Config::GetGlobalConfig()->m_log_file_name + "_app",
  Config::GetGlobalConfig()->m_log_file_path,
  Config::GetGlobalConfig()->m_log_max_file_size);
}

void Logger::pushLog(const std::string &msg)
{
  ScopeMutex<Mutex> lock(m_mutex);
  m_buffer.push_back(msg);
  lock.unlock();
}

void Logger::pushAppLog(const std::string &msg) {
  ScopeMutex<Mutex> lock(m_app_mutex);
  m_app_buffer.push_back(msg);
  lock.unlock();
}

void Logger::log() {
}

void Logger::syncloop() {
  // 同步 m_buffer 到 async_logger 的 buffer 队尾
  std::vector<std::string> tmp_vec;
  ScopeMutex<Mutex> locker(m_mutex);
  tmp_vec.swap(m_buffer);
  locker.unlock();

  if (!tmp_vec.empty()) {
    m_async_logger->pushLogBuffer(tmp_vec);
  }

  std::vector<std::string> tmp_app_vec;
  ScopeMutex<Mutex> app_locker(m_app_mutex);
  tmp_app_vec.swap(m_app_buffer);
  app_locker.unlock();

  if (!tmp_app_vec.empty()) {
    m_async_app_logger->pushLogBuffer(tmp_app_vec);
  }
}

void Logger::init() {

  m_timer_event = std::make_shared<TimerEvent>(Config::GetGlobalConfig()->m_log_sync_inteval, true, std::bind(&Logger::syncloop, this));
  EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
}

AsyncLogger::AsyncLogger(const std::string file_name, const std::string file_path, int max_size)
  : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {
  sem_init(&m_sempahore, 0, 0);
  pthread_create(&m_thread, NULL, &AsyncLogger::loop, this);

  sem_wait(&m_sempahore);
  
}

void* AsyncLogger::loop(void* arg) {
  // 将 buffer 里面的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程
  AsyncLogger* logger = (AsyncLogger*) arg;
  pthread_cond_init(&logger->m_cond, NULL);

  sem_post(&logger->m_sempahore);

  while (1) {
    ScopeMutex<Mutex> locker(logger->m_mutex);
    while (logger->m_buffer.empty()) { // 这里一定要是 while, 为什么？
      pthread_cond_wait(&(logger->m_cond), logger->m_mutex.getMutex());
    }

    std::vector<std::string> tmp;
    tmp.swap(logger->m_buffer.front());
    logger->m_buffer.pop();
    locker.unlock();

    timeval now;
    gettimeofday(&now, NULL);

    struct tm now_time;
    localtime_r(&(now.tv_sec), &now_time);

    const char* format = "%Y%m%d";
    char date[32];
    strftime(date, sizeof(date), format, &now_time);

    if (std::string(date) != logger->m_date) {
      logger->m_no = 0;
      logger->m_reopen_flag = true;
      logger->m_date = std::string(date);
    }

    if (logger->m_file_handler == NULL) {
      logger->m_reopen_flag = true;
    }

    std::stringstream ss;
    ss << logger->m_file_path << logger->m_file_name << "_"
      << std::string(date) << "_log.";
    std::string log_file_name = ss.str() + std::to_string(logger->m_no);

    if (logger->m_reopen_flag) {
      if (logger->m_file_handler) {
        fclose(logger->m_file_handler);
      }
      logger->m_file_handler = fopen(log_file_name.c_str(), "a");
      if (!logger->m_file_handler) {
        printf("fopen, error info [%s]\n", strerror(errno));
        printf("log file name [%s]\n", log_file_name.c_str());
        return NULL;
      }
      // TODO 添加一个打开文件的错误判断
      logger->m_reopen_flag = false;
    }

    if (ftell(logger->m_file_handler) > logger->m_max_file_size) {
      fclose(logger->m_file_handler);

      log_file_name = ss.str() + std::to_string(logger->m_no++);
      logger->m_file_handler = fopen(log_file_name.c_str(), "a");
      logger->m_reopen_flag = false;
    }

    for (auto& i : tmp) {
      if (!i.empty()) {
        fwrite(i.c_str(), 1, i.length(), logger->m_file_handler);
      }
    }
    fflush(logger->m_file_handler);

    if (logger->m_stop_falg) {
      fclose(logger->m_file_handler);
      return NULL;
    }
  }
  
  return NULL;
}

void AsyncLogger::stop() {
  m_stop_falg = true;
}

void AsyncLogger::flush() {
  if (m_file_handler) {
    fflush(m_file_handler);
  }
}

void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec) {
  ScopeMutex<Mutex> locker(m_mutex);
  m_buffer.push(vec);
  locker.unlock();

  // 这时候需要唤醒异步日志线程
  pthread_cond_signal(&m_cond);
}


}
