#include "base_protocol.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include <arpa/inet.h>
#include <sys/socket.h>

BaseProtocol::BaseProtocol(Socket&& socket): socket(std::move(socket)) {}

bool BaseProtocol::send_u8(uint8_t value) { return socket.sendall(&value, sizeof(value)) != 0; }

bool BaseProtocol::send_u16(uint16_t value) {
    uint16_t net = htons(value);
    return socket.sendall(&net, sizeof(net)) != 0;
}

bool BaseProtocol::send_string(const std::string& value) {
    uint16_t len = static_cast<uint16_t>(value.size());
    if (!send_u16(len))
        return false;
    if (len == 0)
        return true;
    return socket.sendall(value.data(), len) != 0;
}

std::optional<uint8_t> BaseProtocol::recv_u8() {
    uint8_t value;
    if (socket.recvall(&value, sizeof(value)) == 0)
        return std::nullopt;
    return value;
}

std::optional<uint16_t> BaseProtocol::recv_u16() {
    uint16_t net;
    if (socket.recvall(&net, sizeof(net)) == 0)
        return std::nullopt;
    return ntohs(net);
}

std::optional<std::string> BaseProtocol::recv_string() {
    std::optional<uint16_t> len = recv_u16();
    if (!len)
        return std::nullopt;
    if (*len == 0)
        return std::string();
    std::string value(*len, '\0');
    if (socket.recvall(value.data(), *len) == 0)
        return std::nullopt;
    return value;
}

void BaseProtocol::shutdown() { socket.shutdown(SHUT_RDWR); }
