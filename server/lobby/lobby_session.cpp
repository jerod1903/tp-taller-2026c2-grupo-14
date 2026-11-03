#include "server/lobby/lobby_session.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

#include "common/appearance_catalog.h"
#include "common/logger.h"
#include "common/net/liberror.h"
#include "server/consts.h"

namespace {

bool is_valid_name_char(unsigned char c) { return std::isalnum(c) || c == '_' || c == '-'; }

bool is_valid_name(const std::string& name) {
    return !name.empty() && name.size() <= ServerConstants::MAX_NAME_LEN &&
           std::all_of(name.begin(), name.end(), is_valid_name_char);
}

// El nombre de una partida además admite espacios entre palabras.
bool is_valid_game_name(const std::string& name) {
    return !name.empty() && name.size() <= ServerConstants::MAX_NAME_LEN && name.front() != ' ' &&
           name.back() != ' ' && std::all_of(name.begin(), name.end(), [](unsigned char c) {
               return is_valid_name_char(c) || c == ' ';
           });
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
                {LobbyOp::CheckInviteCode, [this]() { return handle_check_invite_code(); }},
                {LobbyOp::CreateGame, [this]() { return handle_create_game(); }},
                {LobbyOp::JoinGame, [this]() { return handle_join_game(); }},
                {LobbyOp::LoadGame, [this]() { return handle_load_game(); }},
                {LobbyOp::EnableMultiplayer, [this]() { return handle_enable_multiplayer(); }},
        } {}

void LobbySession::start() {
    Thread::start();
    started = true;
}

void LobbySession::run() {
    try {
        while (should_keep_running() && !username && handle_login()) {}
        while (should_keep_running() && username && handle_next_operation()) {}
    } catch (const LibError& e) {
        // El cliente cortó a mitad de un mensaje.
        Logger::debug("conexión cortada: {}", e.what());
    }
    if (current_game) {
        registry.leave_game(*current_game, *username);
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
        Logger::warn("login rechazado ({}): user '{}' versión {}", to_string(status),
                     printable(req->username), static_cast<unsigned>(req->protocolVersion));
        // Con otra versión reintentar no sirve; un nombre inválido se puede corregir.
        return status != LobbyStatus::ProtocolMismatch;
    }
    Logger::info("login: user '{}'", req->username);
    username = std::move(req->username);
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
    return protocol.send_saved_game_list(registry.games_of(*username));
}

bool LobbySession::handle_check_invite_code() {
    std::optional<CheckInviteCodeRequest> req = protocol.recv_check_invite_code();
    if (!req) {
        return false;
    }
    return protocol.send_status(registry.check_invite_code(req->inviteCode, *username));
}

bool LobbySession::handle_create_game() {
    std::optional<CreateGameRequest> req = protocol.recv_create_game();
    if (!req) {
        return false;
    }
    if (!is_valid_game_name(req->gameName) || !is_valid_name(req->characterName)) {
        return protocol.send_game_entered(GameEnteredResponse{LobbyStatus::InvalidName, 0, 0});
    }
    if (!AppearanceCatalog::is_valid(req->appearance)) {
        return protocol.send_game_entered(
                GameEnteredResponse{LobbyStatus::InvalidAppearance, 0, 0});
    }
    std::optional<uint32_t> game_id = registry.create_game(
            req->gameName, req->worldId,
            GameMember{*username, std::move(req->characterName), req->appearance});
    if (!game_id) {
        return protocol.send_game_entered(GameEnteredResponse{LobbyStatus::WorldNotFound, 0, 0});
    }
    Logger::info("user '{}' creó la partida {} '{}'", *username, *game_id, req->gameName);
    enter_game(*game_id);
    return protocol.send_game_entered(
            GameEnteredResponse{LobbyStatus::Ok, *game_id, GameRegistry::OWNER_PLAYER_ID});
}

bool LobbySession::handle_join_game() {
    std::optional<JoinGameRequest> req = protocol.recv_join_game();
    if (!req) {
        return false;
    }
    if (!is_valid_name(req->characterName)) {
        return protocol.send_game_entered(GameEnteredResponse{LobbyStatus::InvalidName, 0, 0});
    }
    if (!AppearanceCatalog::is_valid(req->appearance)) {
        return protocol.send_game_entered(
                GameEnteredResponse{LobbyStatus::InvalidAppearance, 0, 0});
    }
    GameEnteredResponse response = registry.join_game(
            req->inviteCode, GameMember{*username, std::move(req->characterName), req->appearance});
    if (response.status == LobbyStatus::Ok) {
        Logger::info("user '{}' se unió a la partida {}", *username, response.gameId);
        enter_game(response.gameId);
    }
    return protocol.send_game_entered(response);
}

bool LobbySession::handle_load_game() {
    std::optional<LoadGameRequest> req = protocol.recv_load_game();
    if (!req) {
        return false;
    }
    // Volver a pedir la partida en la que ya está no la suelta: responde AlreadyConnected.
    GameEnteredResponse response = registry.load_game(req->gameId, *username);
    if (response.status == LobbyStatus::Ok) {
        Logger::info("user '{}' retomó la partida {}", *username, req->gameId);
        enter_game(req->gameId);
    }
    return protocol.send_game_entered(response);
}

bool LobbySession::handle_enable_multiplayer() {
    std::optional<EnableMultiplayerRequest> req = protocol.recv_enable_multiplayer();
    if (!req) {
        return false;
    }
    EnableMultiplayerResponse response = registry.enable_multiplayer(req->gameId, *username);
    if (response.status == LobbyStatus::Ok) {
        Logger::info("user '{}' abrió la partida {} al multijugador", *username, req->gameId);
    }
    return protocol.send_enable_multiplayer(response);
}

void LobbySession::enter_game(uint32_t game_id) {
    if (current_game) {
        registry.leave_game(*current_game, *username);
    }
    current_game = game_id;
}

LobbyStatus LobbySession::validate(const LoginRequest& req) const {
    if (req.protocolVersion != PROTOCOL_VERSION) {
        return LobbyStatus::ProtocolMismatch;
    }
    if (!is_valid_name(req.username)) {
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
