#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <mosquitto.h>
#include "ICommand.h"
namespace stago {

class MQTTMngr : public ICommand
{
public:
    typedef std::function<void(void *)> mqttCb_t;

public:
    MQTTMngr(const char * client_name, const char * address, uint16_t port);
    virtual ~MQTTMngr();

    int connect() override;
    int disconnect() override;
    int process(uint32_t events) override;
    int keepAlive() override { return this->misc(); }

    bool isConnected() const override { return (m_client != nullptr) && (getFd() > 0); }
    int getFd() const override { return ::mosquitto_socket(m_client); }

protected:
    MQTTMngr(const MQTTMngr &) = delete;
    MQTTMngr & operator=(const MQTTMngr &) = delete;

    void createClient();
    virtual void dispatchMsg(const struct mosquitto_message & msg);
    virtual int initCallback() = 0;

    int misc() { return ::mosquitto_loop_misc(m_client); }
    int read(int mp) { return ::mosquitto_loop_read(m_client, mp); }
    int write(int mp) { return ::mosquitto_loop_write(m_client, mp); }

    std::string m_clientName;

    struct mosquitto * m_client;
    std::string m_addr;
    int16_t m_port;

    std::map<std::string, mqttCb_t> m_cbs;
};

class BCMQTTMngr : public MQTTMngr
{
public:
    static constexpr const char * bareCodeTopic = "barecode/event/processImage/+";

    typedef std::function<void(char *path, size_t len)>
            processImageCb_t;

public:
    BCMQTTMngr(const char * clientName, const char * address, uint16_t port,
               processImageCb_t processImageCb);

private:
    int initCallback() override;
    void dispatchMsg(const struct mosquitto_message & msg) override;

    processImageCb_t m_processImageCb;
};

}; /*namespace stago*/