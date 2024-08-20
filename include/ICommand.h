#ifndef SRC_ICOMMAND_H_
#define SRC_ICOMMAND_H_

#include <cstdint>

namespace stago {
class ICommand
{
 public:
    virtual ~ICommand() {}
    virtual bool isConnected() const = 0;
    virtual int getFd() const = 0;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual int process(uint32_t events) = 0;
    virtual int keepAlive() = 0;
};
}; /* namespace stago */

#endif /* SRC_ICOMMAND_H_ */