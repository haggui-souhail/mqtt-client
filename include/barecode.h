#include <sys/epoll.h>

#include "MQTTMngr.h"

namespace stago {
class barecode
{
private:
    static int epollAddClient(int epollFd, struct epoll_event * epollEvent, int fd);
public:
    barecode(/* args */);
    ~barecode();
    int runloop(BCMQTTMngr & mqttMngr);
    int mqtt_bare_code_cb(char *path, size_t size);
};


}; /* name space stago*/