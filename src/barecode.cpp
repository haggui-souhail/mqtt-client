#define ULOG_TAG BC_BASE
#include <ulog.h>
#include "barecode.h"
#include <string.h>



#include "MQTTMngr.h"

ULOG_DECLARE_TAG(BC_BASE);

namespace stago {
barecode::barecode()
{
}

barecode::~barecode()
{
}

int barecode::epollAddClient(int epollFd, struct epoll_event * ev, int fd)
{
    ev->data.fd = fd;
    ev->events = EPOLLIN | EPOLLRDHUP;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, ev) < 0) {
        ULOGE("%s: epoll_ctl() failed: %s", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

int barecode::runloop(BCMQTTMngr & mqttMngr)
{
    int n;
    struct epoll_event ev;

    int epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd == -1) {
        ULOGE("[%s:%u] epoll_create1() failed: %s", __func__, __LINE__, ::strerror(errno));
        return -1;
    }

    if (mqttMngr.connect() == MOSQ_ERR_SUCCESS) {
        this->epollAddClient(epollFd, &ev, mqttMngr.getFd());
    }

    bool loop = true;
    ULOGI("[%s:%u] starting main loop", __func__, __LINE__);
    while (loop) {
        n = epoll_wait(epollFd, &ev, 1, -1);
        if ((n < 0) && (errno == EINTR)) {
            continue;
        }
        if (n < 0) {
            ULOGE("[%s:%u] epoll_wait() failed: %s", __func__, __LINE__, ::strerror(errno));
            break;
        }

        if (!mqttMngr.isConnected()) {
            if (mqttMngr.connect() == MOSQ_ERR_SUCCESS) {
                this->epollAddClient(epollFd, &ev, mqttMngr.getFd());
            }
        }
        if (n == 0) {
            continue;
        }        
        if (ev.data.fd == mqttMngr.getFd()) {
            ULOGE("[%s:%u] events for mosquitto: %d", __func__, __LINE__, ev.events);
            mqttMngr.process(ev.events);
        }

    }

    return 0;   
}

int barecode::mqtt_bare_code_cb(char *path, size_t size)
{
    ULOGI("%s\r\n", path);
    return 0;
}
} /* namespace stago */
