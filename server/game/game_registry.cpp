#include "server/game/game_registry.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr uint8_t SINGLE_PLAYER_MAX = 1;

}  // namespace

GameRegistry::GameRegistry(std::vector<WorldInfo> worlds, GameStore& store, uint8_t max_players):
        worlds(std::move(worlds)), max_players(max_players), store(store) {
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
    return game.gameId;
}

CheckInviteCodeResponse GameRegistry::check_invite_code(const std::string& code,
                                                        const std::string& username) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<std::string, uint32_t>::const_iterator id = games_by_code.find(code);
    if (id == games_by_code.end()) {
        return CheckInviteCodeResponse{LobbyStatus::InvalidInviteCode, false, 0};
    }
    const GameRecord& game = games.at(id->second);
    if (is_member(game, username)) {
        return CheckInviteCodeResponse{LobbyStatus::Ok, true, game.gameId};
    }
    if (game.members.size() >= game.maxPlayers) {
        return CheckInviteCodeResponse{LobbyStatus::GameFull, false, 0};
    }
    return CheckInviteCodeResponse{LobbyStatus::Ok, false, 0};
}

JoinGameResponse GameRegistry::join_game(const std::string& code, const GameMember& member) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<std::string, uint32_t>::const_iterator id = games_by_code.find(code);
    if (id == games_by_code.end()) {
        return JoinGameResponse{LobbyStatus::InvalidInviteCode, 0};
    }
    GameRecord& game = games.at(id->second);
    if (is_member(game, member.username)) {
        return JoinGameResponse{LobbyStatus::AlreadyMember, 0};
    }
    if (game.members.size() >= game.maxPlayers) {
        return JoinGameResponse{LobbyStatus::GameFull, 0};
    }
    if (name_taken(game, member.characterName)) {
        return JoinGameResponse{LobbyStatus::NameTaken, 0};
    }
    game.members.push_back(member);
    store.save(game);
    games_by_user[member.username].insert(game.gameId);
    connected[game.gameId].insert(member.username);
    return JoinGameResponse{LobbyStatus::Ok, game.gameId};
}

LobbyStatus GameRegistry::load_game(uint32_t game_id, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<uint32_t, GameRecord>::const_iterator game = games.find(game_id);
    if (game == games.end() || !is_member(game->second, username)) {
        return LobbyStatus::GameNotFound;
    }
    if (!connected[game_id].insert(username).second) {
        return LobbyStatus::AlreadyConnected;
    }
    return LobbyStatus::Ok;
}

void GameRegistry::leave_game(uint32_t game_id, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<uint32_t, std::set<std::string>>::iterator users = connected.find(game_id);
    if (users == connected.end()) {
        return;
    }
    users->second.erase(username);
    if (users->second.empty()) {
        connected.erase(users);
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

bool GameRegistry::is_member(const GameRecord& game, const std::string& username) {
    return std::any_of(game.members.begin(), game.members.end(),
                       [&username](const GameMember& m) { return m.username == username; });
}

bool GameRegistry::name_taken(const GameRecord& game, const std::string& character_name) {
    return std::any_of(
            game.members.begin(), game.members.end(),
            [&character_name](const GameMember& m) { return m.characterName == character_name; });
}
