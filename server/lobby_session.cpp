#include "lobby_session.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>
#include <utility>

#include "common/logger.h"
#include "common/net/liberror.h"

#include "consts.h"

namespace {

bool is_valid_uuid_v4(const UserId& id) { return (id[6] >> 4) == 0x4 && (id[8] & 0xC0) == 0x80; }

bool is_valid_name_char(unsigned char c) { return std::isalnum(c) || c == '_' || c == '-'; }

bool is_valid_name(const std::string& name) {
    return !name.empty() && name.size() <= ServerConstants::MAX_NAME_LEN &&
           std::all_of(name.begin(), name.end(), is_valid_name_char);
}

// Formato canónico 8-4-4-4-12, el mismo que muestra el cliente.
std::string format_user_id(const UserId& id) {
    std::string out;
    for (std::size_t i = 0; i < id.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out += '-';
        }
        out += std::format("{:02x}", id[i]);
    }
    return out;
}

// El nombre de un login rechazado viene sin validar: se trunca y se reemplazan los caracteres
// no imprimibles para que no pueda inyectar líneas en el log.
std::string printable(const std::string& text) {
    std::string out;
    for (std::size_t i = 0; i < text.size() && i < ServerConstants::MAX_LOGGED_LEN; ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        out += std::isprint(c) ? text[i] : '?';
    }
    if (text.size() > ServerConstants::MAX_LOGGED_LEN) {
        out += "...";
    }
    return out;
}

}  // namespace

LobbySession::LobbySession(Socket&& peer, GameRegistry& registry):
        protocol(std::move(peer)),
        registry(registry),
        handlers{
                {LobbyOp::ListWorlds, [this]() { return handle_list_worlds(); }},
                {LobbyOp::ListMyGames, [this]() { return handle_list_my_games(); }},
        } {}

void LobbySession::start() {
    Thread::start();
    started = true;
}

void LobbySession::run() {
    try {
        if (handle_login()) {
            while (should_keep_running() && handle_next_operation()) {}
        }
    } catch (const LibError& e) {
        // El cliente cortó a mitad de un mensaje.
        Logger::debug("conexión cortada: {}", e.what());
    }
    // Se cierra acá y no en el destructor: el Acceptor recién lo destruye en el próximo accept.
    // Si se está deteniendo, el shutdown ya lo hizo el destructor.
    if (should_keep_running()) {
        try {
            protocol.shutdown();
        } catch (const LibError&) {}
    }
}

bool LobbySession::handle_login() {
    std::optional<LobbyOp> op = protocol.recv_operation();
    if (!op) {
        Logger::debug("el cliente cerró antes del login");
        return false;
    }
    if (*op != LobbyOp::Login) {
        Logger::debug("operación {} antes del login", static_cast<unsigned>(*op));
        return false;
    }
    std::optional<LoginRequest> req = protocol.recv_login();
    if (!req) {
        Logger::debug("el cliente cerró a mitad del login");
        return false;
    }
    LobbyStatus status = validate(*req);
    if (!protocol.send_status(status)) {
        return false;
    }
    if (status != LobbyStatus::Ok) {
        Logger::warn("login rechazado ({}): user {} '{}' versión {}", to_string(status),
                     format_user_id(req->userId), printable(req->username),
                     static_cast<unsigned>(req->protocolVersion));
        return false;
    }
    Logger::info("login: user {} '{}'", format_user_id(req->userId), req->username);
    identity = SessionIdentity{req->userId, std::move(req->username)};
    return true;
}

bool LobbySession::handle_next_operation() {
    std::optional<LobbyOp> op = protocol.recv_operation();
    if (!op) {
        Logger::debug("el cliente cerró la conexión");
        return false;
    }
    HandlerMap::const_iterator handler = handlers.find(*op);
    if (handler == handlers.end()) {
        Logger::debug("operación {} inválida en el lobby", static_cast<unsigned>(*op));
        return false;
    }
    return handler->second();
}

bool LobbySession::handle_list_worlds() { return protocol.send_world_list(registry.list_worlds()); }

bool LobbySession::handle_list_my_games() {
    // games_of devuelve una copia: el lock del registry ya se soltó cuando se envía.
    return protocol.send_saved_game_list(registry.games_of(identity->userId));
}

LobbyStatus LobbySession::validate(const LoginRequest& req) const {
    if (req.protocolVersion != PROTOCOL_VERSION) {
        return LobbyStatus::ProtocolMismatch;
    }
    if (!is_valid_uuid_v4(req.userId) || !is_valid_name(req.username)) {
        return LobbyStatus::InvalidName;
    }
    return LobbyStatus::Ok;
}

LobbySession::~LobbySession() {
    stop();
    try {
        protocol.shutdown();
    } catch (...) {}
    if (started) {
        join();
    }
}
