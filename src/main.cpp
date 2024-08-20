#define ULOG_TAG FA_APP
#include <ulog.h>

#include "MQTTMngr.h"
#include "barecode.h"

#define BARECODE_CLIENT "barecode_client"

ULOG_DECLARE_TAG(FA_APP);

int main(int argc, char * argv[])
{
    ULOG_INIT(FA_APP);
    ULOG_SET_LEVEL(ULOG_DEBUG);
    ULOGI("MQTT Client APPLICATION - %s %s\n", __DATE__, __TIME__);
    stago::barecode bc;

    if (argc < 2) {
        ULOGE("help : mqtt_client <ip@>");
        return -1;
    }

    stago::BCMQTTMngr bcMqttMngr(
            BARECODE_CLIENT, argv[1], 1883,
            [&bc](char *path, size_t len) -> int {
                return bc.mqtt_bare_code_cb(path, len);
            });
    bc.runloop(bcMqttMngr);

    ULOGI("Bye Bye !!!\n");

    return 0;
}
