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

std::vector<SavedGameInfo> GameRegistry::games_of(const UserId& user_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<SavedGameInfo> result;
    std::map<UserId, std::set<uint32_t>>::const_iterator ids = games_by_user.find(user_id);
    if (ids == games_by_user.end()) {
        return result;
    }
    result.reserve(ids->second.size());
    for (uint32_t game_id: ids->second) {
        const GameRecord& game = games.at(game_id);
        std::vector<GameMember>::const_iterator member =
                std::find_if(game.members.begin(), game.members.end(),
                             [&user_id](const GameMember& m) { return m.userId == user_id; });
        result.push_back(SavedGameInfo{game.gameId, game.gameName, world_name(game.worldId),
                                       member->characterName, member->appearance,
                                       game.isMultiplayer});
    }
    return result;
}

std::optional<uint32_t> GameRegistry::create_game(const std::string& game_name,
                                                  const std::string& world_id, bool is_multiplayer,
                                                  const GameMember& owner) {
    if (!world_exists(world_id)) {
        return std::nullopt;
    }
    std::lock_guard<std::mutex> lock(mutex);
    GameRecord game{next_game_id++,
                    game_name,
                    world_id,
                    is_multiplayer,
                    is_multiplayer ? max_players : SINGLE_PLAYER_MAX,
                    {owner}};
    store.save(game);
    add(game);
    return game.gameId;
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
        games_by_user[member.userId].insert(game.gameId);
    }
    games.insert_or_assign(game.gameId, game);
}
