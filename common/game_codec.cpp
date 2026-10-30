#include "common/game_codec.h"

#include <array>
#include <stdexcept>
#include <string>
#include <variant>

namespace {



// El opcode lo escribe cada write(), asi el formato de un mensaje queda en un solo lugar.
bool write(BaseProtocol& p, const PlayerState& s) {
    return p.send_u8(static_cast<uint8_t>(GameOp::State)) && p.send_u32(s.id) &&
           p.send_u32(s.x) && p.send_u32(s.y) && p.send_u32(s.life) &&
           p.send_u32(s.energy) && p.send_u32(s.coins);
}


// Los read_* leen solo los campos: el opcode ya lo consumio el despachador.
std::optional<MoveCommand> read_move(BaseProtocol& p) {
    std::optional<uint8_t> direction = p.recv_u8();
    if (!direction) {
        return std::nullopt;
    }
    if (*direction > static_cast<uint8_t>(Direction::Right)) {
        throw std::runtime_error("Direccion invalida: " + std::to_string(*direction));
    }
    return MoveCommand{static_cast<Direction>(*direction)};
}

bool write(BaseProtocol& p, const MoveCommand& m) {
    return p.send_u8(static_cast<uint8_t>(GameOp::Move)) &&
           p.send_u8(static_cast<uint8_t>(m.direction));
}

std::optional<PlayerState> read_state(BaseProtocol& p) {
    PlayerState s{};
    for (uint32_t* field: {&s.id, &s.x, &s.y, &s.life, &s.energy, &s.coins}) {
        std::optional<uint32_t> value = p.recv_u32();
        if (!value) {
            return std::nullopt;
        }
        *field = *value;
    }
    return s;
}

}  // namespace

bool write_message(BaseProtocol& p, const ClientMessage& msg) {
    return std::visit([&p](const auto& m) { return write(p, m); }, msg);
}

bool write_message(BaseProtocol& p, const ServerMessage& msg) {
    return std::visit([&p](const auto& m) { return write(p, m); }, msg);
}

std::optional<ClientMessage> read_client_message(BaseProtocol& p) {
    std::optional<uint8_t> op = p.recv_u8();
    if (!op) {
        return std::nullopt;
    }
    switch (static_cast<GameOp>(*op)) {
        case GameOp::Move: {
            std::optional<MoveCommand> msg = read_move(p);
            if (!msg) {
                return std::nullopt;
            }
            return ClientMessage{*msg};
        }
        default:
            throw std::runtime_error("Operacion de cliente invalida: " + std::to_string(*op));
    }
}

std::optional<ServerMessage> read_server_message(BaseProtocol& p) {
    std::optional<uint8_t> op = p.recv_u8();
    if (!op) {
        return std::nullopt;
    }
    switch (static_cast<GameOp>(*op)) {
        case GameOp::State: {
            std::optional<PlayerState> msg = read_state(p);
            if (!msg) {
                return std::nullopt;
            }
            return ServerMessage{*msg};
        }
        default:
            throw std::runtime_error("Operacion de servidor invalida: " + std::to_string(*op));
    }
}