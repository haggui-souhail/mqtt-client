#define ULOG_TAG FASTCAM_MQTTMNGR
#include <ulog.h>

#include "MQTTMngr.h"
#include <sys/epoll.h>
#include <string.h>

using namespace stago;

#define MQTT_SERVER_ADDR "127.0.0.1"
#define MQTT_SERVER_PORT 1883

ULOG_DECLARE_TAG(ULOG_TAG);

MQTTMngr::MQTTMngr(const char * clientName, const char * address, uint16_t port) :
    m_clientName(clientName),
    m_client(nullptr),
    m_addr(address),
    m_port(port){}

    MQTTMngr::~MQTTMngr()
{
    ::mosquitto_destroy(m_client);
}

void MQTTMngr::createClient()
{
    if (m_client != nullptr) {
        ULOGW("mosquitto client already created");
        return;
    }

    ULOGI("Create mqtt client '%s'", m_clientName.c_str());
    m_client = ::mosquitto_new(m_clientName.c_str(), true, this);
    if (m_client == nullptr) {
        ULOGE("[%s:%u]  mosquitto_new() failed", __func__, __LINE__);
    }
}

int MQTTMngr::connect()
{
    int ret;

    if (m_client == nullptr)
        this->createClient();

    ULOGI("Connecting to mqtt (address: %s:%u) ...", m_addr.c_str(), m_port);
    int keep_alive = 0;
    ret = ::mosquitto_connect(m_client, m_addr.c_str(), m_port, keep_alive);
    if (ret != MOSQ_ERR_SUCCESS) {
        ULOGE("[%s:%u]  mosquitto_connect failed: %u", __func__, __LINE__, ret);
        return ret;
    }

    ::mosquitto_message_callback_set(m_client, [](auto mosq, auto data, auto msg) -> void {
        auto * mngr = static_cast<MQTTMngr *>(data);
        mngr->dispatchMsg(*msg);
    });

    this->initCallback();

    ULOGI("mosquitto client has been successfully created");
    return 0;
}

int MQTTMngr::disconnect()
{
    int ret;

    ret = ::mosquitto_disconnect(m_client);
    if (ret != MOSQ_ERR_SUCCESS) {
        ULOGE("[%s:%u]  mosquitto_disconnect_async failed: %u", __func__, __LINE__, ret);
        return ret;
    }

    return ret;
}

int MQTTMngr::process(uint32_t events)
{
    int ret = 0;
    ULOGD("[%s:%u] events for mosquitto: %d", __func__, __LINE__, events);
    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        ULOGW("[%s:%u] error on mosquitto fd %d",
              __func__, __LINE__, this->getFd());
        return -1;
    }

    if (events & EPOLLIN) {
        ret = this->read(1);
        if (ret != MOSQ_ERR_SUCCESS) {
            ULOGE("[%s:%u] mosquitto_loop_read: %d", __func__, __LINE__, ret);
        }
    }

    if (events & EPOLLOUT) {
        ret = this->write(1);
        if (ret != MOSQ_ERR_SUCCESS) {
            ULOGE("[%s:%u] mosquitto_loop_write: %d", __func__, __LINE__, ret);
        }
    }
    return ret;
}

void MQTTMngr::dispatchMsg(const struct mosquitto_message & msg)
{
    ULOGD("receive message   \n"
          "{                 \n"
          "  id         : %d \n"
          "  topic      : %s \n"
          "  payload    : %p \n"
          "  payloadlen : %d \n"
          "  qos        : %d \n"
          "  retain     : %d \n"
          "}",
          msg.mid, msg.topic, msg.payload, msg.payloadlen, msg.qos, msg.retain);
    char * payload = (char *)::malloc(msg.payloadlen + 1);
    ::memcpy(payload, msg.payload, msg.payloadlen);
    payload[msg.payloadlen] = 0;
    ULOGI("topic/msg : '%s':'%s'", msg.topic, payload);

    for (auto cb : m_cbs) {
        if (strcmp(cb.first.c_str(), msg.topic) == 0) {
            cb.second(payload);
        }
    }

    ::free(payload);
}


////////////////////////////////////////////////////////////////////////////////

BCMQTTMngr::BCMQTTMngr(const char * clientName, const char * address, uint16_t port,
                       processImageCb_t processImageCb) :
    MQTTMngr(clientName, address, port), m_processImageCb(processImageCb)
{}

int BCMQTTMngr::initCallback()
{
    int ret = ::mosquitto_subscribe(m_client, NULL, bareCodeTopic, 2);
    if (ret != MOSQ_ERR_SUCCESS) {
        ULOGE("[%s:%u] mosquitto_subscribe '%s' failed: %u", __func__, __LINE__, bareCodeTopic, ret);
        return ret;
    }
    ULOGI("subscribe to mqtt topic '%s'", bareCodeTopic);
    return 0;
}

void BCMQTTMngr::dispatchMsg(const struct mosquitto_message & msg)
{

    ULOGD("receive message   \n"
          "{                 \n"
          "  id         : %d \n"
          "  topic      : %s \n"
          "  payload    : %p \n"
          "  payloadlen : %d \n"
          "  qos        : %d \n"
          "  retain     : %d \n"
          "}",
          msg.mid, msg.topic, msg.payload, msg.payloadlen, msg.qos, msg.retain);

    char *path = strdup((const char*)msg.payload);
    if (!path) {
        ULOGE("[%s:%u] failed to parse rear radar message", __func__, __LINE__);
    } else {
        m_processImageCb(path, strlen(path));
    }

    free(path);
}
