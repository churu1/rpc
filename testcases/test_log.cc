#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include "rocket/common/log.h"
#include "rocket/common/config.h"

void* func(void*) {
  DEBUGLOG("DEBUG this is thread in %s", "foo");
  INFOLOG("INFO this is thread in %s", "foo");
  return NULL;
}


int main() {

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");
  rocket::Logger::InitGlobalLogger();

  pthread_t thread;
  pthread_create(&thread, NULL, &func, NULL);

  DEBUGLOG("DEBUG test log %s", "cpper");
  INFOLOG("INFO test log %s", "cpper");
  pthread_join(thread, NULL);
  return 0;
}