#ifndef BASE_PROTOCOL_H
#define BASE_PROTOCOL_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "net/socket.h"

class BaseProtocol {
public:
    explicit BaseProtocol(Socket&& socket);

    BaseProtocol(const BaseProtocol&) = delete;
    BaseProtocol& operator=(const BaseProtocol&) = delete;
    BaseProtocol(BaseProtocol&&) = default;
    BaseProtocol& operator=(BaseProtocol&&) = default;

    bool send_u8(uint8_t value);
    bool send_u16(uint16_t value);
    bool send_string(const std::string& value);
    bool send_bytes(const uint8_t* data, std::size_t size);

    std::optional<uint8_t> recv_u8();
    std::optional<uint16_t> recv_u16();
    std::optional<std::string> recv_string();
    bool recv_bytes(uint8_t* data, std::size_t size);

    void shutdown();

private:
    Socket socket;
};

#endif  // BASE_PROTOCOL_H
