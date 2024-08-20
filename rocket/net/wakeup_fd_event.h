#ifndef ROCKET_NET_WAKEUP_FDEVENT
#define ROCKET_NET_WAKEUP_FDEVENT

#include "rocket/net/fd_event.h"

namespace rocket {
 class WakeUpFdEvent : public FdEvent {
  public:
   WakeUpFdEvent(int fd);

   ~WakeUpFdEvent();

   void wakeup();
  private:
 
 };

}

#endif