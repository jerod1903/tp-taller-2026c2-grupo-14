#include "server/game/game_registry.h"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "common/logger.h"

namespace {

constexpr uint8_t SINGLE_PLAYER_MAX = 1;

constexpr std::size_t INVITE_CODE_LEN = 6;
// Sin 0/O, 1/I, 2/Z, 5/S ni 8/B: el código se dicta y se tipea a mano, y en la fuente pixelada
// del chat esos pares se confunden.
constexpr std::string_view INVITE_CODE_CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ34679";

std::vector<WorldInfo> infos_of(const std::vector<GameWorld>& worlds) {
    std::vector<WorldInfo> infos;
    for (const GameWorld& world: worlds) {
        infos.push_back(world.info);
    }
    return infos;
}

std::map<std::string, stardew::maps::MapDefinition> maps_of(const std::vector<GameWorld>& worlds) {
    std::map<std::string, stardew::maps::MapDefinition> maps;
    for (const GameWorld& world: worlds) {
        maps.emplace(world.info.worldId, world.map);
    }
    return maps;
}

}  // namespace

GameRegistry::GameRegistry(const std::vector<GameWorld>& worlds, GameStore& store,
                           uint8_t max_players):
        worlds(infos_of(worlds)), maps(maps_of(worlds)), max_players(max_players), store(store) {
    for (const GameRecord& game: store.load_all()) {
        add(game);
        next_game_id = std::max(next_game_id, game.gameId + 1);
    }
}

const std::vector<WorldInfo>& GameRegistry::list_worlds() const { return worlds; }

std::vector<SavedGameInfo> GameRegistry::games_of(const std::string& username) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<SavedGameInfo> result;
    std::map<std::string, std::set<uint32_t>>::const_iterator ids = games_by_user.find(username);
    if (ids == games_by_user.end()) {
        return result;
    }
    result.reserve(ids->second.size());
    for (uint32_t game_id: ids->second) {
        const GameRecord& game = games.at(game_id);
        std::vector<GameMember>::const_iterator member =
                std::find_if(game.members.begin(), game.members.end(),
                             [&username](const GameMember& m) { return m.username == username; });
        result.push_back(SavedGameInfo{game.gameId, game.gameName, world_name(game.worldId),
                                       member->characterName, member->appearance,
                                       game.isMultiplayer});
    }
    return result;
}

std::optional<uint32_t> GameRegistry::create_game(const std::string& game_name,
                                                  const std::string& world_id,
                                                  const GameMember& owner) {
    if (!world_exists(world_id)) {
        return std::nullopt;
    }
    std::lock_guard<std::mutex> lock(mutex);
    GameRecord game{next_game_id++, game_name, world_id, false, SINGLE_PLAYER_MAX, {owner}, ""};
    store.save(game);
    add(game);
    connected[game.gameId].insert(owner.username);
    return game.gameId;
}

LobbyStatus GameRegistry::check_invite_code(const std::string& code,
                                            const std::string& username) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<std::string, uint32_t>::const_iterator id = games_by_code.find(code);
    if (id == games_by_code.end()) {
        return LobbyStatus::InvalidInviteCode;
    }
    const GameRecord& game = games.at(id->second);
    if (is_member(game, username)) {
        return LobbyStatus::AlreadyMember;
    }
    if (game.members.size() >= game.maxPlayers) {
        return LobbyStatus::GameFull;
    }
    return LobbyStatus::Ok;
}

GameEnteredResponse GameRegistry::join_game(const std::string& code, const GameMember& member) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<std::string, uint32_t>::const_iterator id = games_by_code.find(code);
    if (id == games_by_code.end()) {
        return GameEnteredResponse{LobbyStatus::InvalidInviteCode, 0, 0};
    }
    GameRecord& game = games.at(id->second);
    if (!world_exists(game.worldId)) {
        return GameEnteredResponse{LobbyStatus::WorldNotFound, 0, 0};
    }
    if (is_member(game, member.username)) {
        return GameEnteredResponse{LobbyStatus::AlreadyMember, 0, 0};
    }
    if (game.members.size() >= game.maxPlayers) {
        return GameEnteredResponse{LobbyStatus::GameFull, 0, 0};
    }
    if (name_taken(game, member.characterName)) {
        return GameEnteredResponse{LobbyStatus::NameTaken, 0, 0};
    }
    if (!launch(game.gameId)) {
        return GameEnteredResponse{LobbyStatus::GameClosed, 0, 0};
    }
    uint32_t player_id = static_cast<uint32_t>(game.members.size());
    game.members.push_back(member);
    store.save(game);
    games_by_user[member.username].insert(game.gameId);
    connected[game.gameId].insert(member.username);
    return GameEnteredResponse{LobbyStatus::Ok, game.gameId, player_id};
}

GameEnteredResponse GameRegistry::load_game(uint32_t game_id, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<uint32_t, GameRecord>::const_iterator game = games.find(game_id);
    std::optional<uint32_t> player_id =
            game == games.end() ? std::nullopt : player_id_of(game->second, username);
    if (!player_id) {
        return GameEnteredResponse{LobbyStatus::GameNotFound, 0, 0};
    }
    if (!world_exists(game->second.worldId)) {
        return GameEnteredResponse{LobbyStatus::WorldNotFound, 0, 0};
    }
    if (connected[game_id].count(username) > 0) {
        return GameEnteredResponse{LobbyStatus::AlreadyConnected, 0, 0};
    }
    if (!launch(game_id)) {
        return GameEnteredResponse{LobbyStatus::GameClosed, 0, 0};
    }
    connected[game_id].insert(username);
    return GameEnteredResponse{LobbyStatus::Ok, game_id, *player_id};
}

GameMember GameRegistry::member_of(uint32_t game_id, uint32_t player_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    return games.at(game_id).members.at(player_id);
}

EnableMultiplayerResponse GameRegistry::enable_multiplayer(uint32_t game_id,
                                                           const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<uint32_t, GameRecord>::iterator found = games.find(game_id);
    if (found == games.end() || !is_member(found->second, username)) {
        return EnableMultiplayerResponse{LobbyStatus::GameNotFound, ""};
    }
    GameRecord& game = found->second;
    if (game.members.front().username != username) {
        return EnableMultiplayerResponse{LobbyStatus::NotOwner, ""};
    }
    if (!game.isMultiplayer) {
        game.isMultiplayer = true;
        game.maxPlayers = max_players;
        game.inviteCode = new_invite_code();
        games_by_code.insert_or_assign(game.inviteCode, game_id);
        store.save(game);
    }
    return EnableMultiplayerResponse{LobbyStatus::Ok, game.inviteCode};
}

Inbox& GameRegistry::attach(uint32_t game_id) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!launch(game_id)) {
        throw std::runtime_error("could not load game " + std::to_string(game_id));
    }
    return running.at(game_id)->get_inbox();
}

bool GameRegistry::is_running(uint32_t game_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    return running.count(game_id) > 0;
}

void GameRegistry::leave_game(uint32_t game_id, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<uint32_t, std::set<std::string>>::iterator users = connected.find(game_id);
    if (users == connected.end()) {
        return;
    }
    users->second.erase(username);
    if (!users->second.empty()) {
        return;
    }
    connected.erase(users);
    std::map<uint32_t, std::unique_ptr<Game>>::iterator game = running.find(game_id);
    if (game != running.end()) {
        game->second->stop();
        // Si quedaba otro detenido de esta partida, al reemplazarlo se espera a que termine
        finished[game_id] = std::move(game->second);
        running.erase(game);
    }
}

GameRegistry::~GameRegistry() {
    // Se detienen todos antes de joinear para no esperar un tick por cada uno.
    for (std::pair<const uint32_t, std::unique_ptr<Game>>& game: running) {
        game.second->stop();
    }
}

bool GameRegistry::world_exists(const std::string& world_id) const {
    return std::any_of(worlds.begin(), worlds.end(),
                       [&world_id](const WorldInfo& world) { return world.worldId == world_id; });
}

std::string GameRegistry::world_name(const std::string& world_id) const {
    std::vector<WorldInfo>::const_iterator world =
            std::find_if(worlds.begin(), worlds.end(),
                         [&world_id](const WorldInfo& w) { return w.worldId == world_id; });
    // Una partida guardada de un mundo que ya no se carga: se muestra el id.
    return world != worlds.end() ? world->name : world_id;
}

void GameRegistry::add(const GameRecord& game) {
    for (const GameMember& member: game.members) {
        games_by_user[member.username].insert(game.gameId);
    }
    if (!game.inviteCode.empty()) {
        games_by_code.insert_or_assign(game.inviteCode, game.gameId);
    }
    games.insert_or_assign(game.gameId, game);
}

std::string GameRegistry::new_invite_code() {
    std::uniform_int_distribution<std::size_t> pick(0, INVITE_CODE_CHARS.size() - 1);
    std::string code;
    do {
        code.clear();
        for (std::size_t i = 0; i < INVITE_CODE_LEN; ++i) {
            code += INVITE_CODE_CHARS[pick(rng)];
        }
    } while (games_by_code.count(code) > 0);
    return code;
}

void GameRegistry::reap() {
    std::erase_if(finished, [](const auto& entry) { return !entry.second->is_alive(); });
}

bool GameRegistry::launch(uint32_t game_id) {
    reap();
    if (running.count(game_id) > 0) {
        return true;
    }
    // El que se estaba cerrando puede estar guardando en su último tick: se espera a que termine
    // (el destructor del Game lo joinea) para cargar lo que haya guardado.
    finished.erase(game_id);
    std::optional<SavedGame> saved;
    try {
        saved = store.load_game(game_id);
    } catch (const std::exception& e) {
        Logger::error("no se pudo cargar la partida {}: {}", game_id, e.what());
        return false;
    }
    const std::string& world_id = games.at(game_id).worldId;
    auto game = std::make_unique<Game>(world_id, maps.at(world_id), game_id, store, saved);
    game->start();
    running.emplace(game_id, std::move(game));
    return true;
}

bool GameRegistry::is_member(const GameRecord& game, const std::string& username) {
    return player_id_of(game, username).has_value();
}

std::optional<uint32_t> GameRegistry::player_id_of(const GameRecord& game,
                                                   const std::string& username) {
    std::vector<GameMember>::const_iterator member =
            std::find_if(game.members.begin(), game.members.end(),
                         [&username](const GameMember& m) { return m.username == username; });
    if (member == game.members.end()) {
        return std::nullopt;
    }
    return static_cast<uint32_t>(member - game.members.begin());
}

bool GameRegistry::name_taken(const GameRecord& game, const std::string& character_name) {
    return std::any_of(
            game.members.begin(), game.members.end(),
            [&character_name](const GameMember& m) { return m.characterName == character_name; });
}
