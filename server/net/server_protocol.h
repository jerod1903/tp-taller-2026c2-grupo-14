#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <optional>
#include <vector>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"

class ServerProtocol {
public:
    explicit ServerProtocol(Socket&& socket);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;

    // std::nullopt si el peer cerró.
    std::optional<LobbyOp> recv_operation();
    std::optional<LoginRequest> recv_login();
    std::optional<CheckInviteCodeRequest> recv_check_invite_code();
    std::optional<CreateGameRequest> recv_create_game();
    std::optional<JoinGameRequest> recv_join_game();
    std::optional<LoadGameRequest> recv_load_game();
    std::optional<EnableMultiplayerRequest> recv_enable_multiplayer();

    bool send_status(LobbyStatus status);
    bool send_world_list(const std::vector<WorldInfo>& worlds);
    bool send_saved_game_list(const std::vector<SavedGameInfo>& games);
    bool send_game_entered(const GameEnteredResponse& response);

    // Durante la partida.
    std::optional<GameOp> recv_game_op();
    std::optional<MoveRequest> recv_move();
    std::optional<SelectSlotRequest> recv_select_slot();
    std::optional<ChatRequest> recv_chat();
    std::optional<TypingRequest> recv_typing();
    std::optional<ClickRequest> recv_click();
    std::optional<ChestSlotRequest> recv_chest_slot();

    bool send_location_data(const LocationData& location);
    bool send_snapshot(const Snapshot& snapshot);
    bool send_game_ended(const GameEnded& ended);
    bool send_player_info(const PlayerInfo& info);
    bool send_inventory_state(const InventoryState& inventory);
    bool send_chat_message(const ChatMessage& message);
    bool send_enable_multiplayer(const EnableMultiplayerResponse& response);
    bool send_farm_state(const FarmState& farm);
    bool send_dungeon_state(const DungeonState& dungeon);
    bool send_day_ended(const DayEnded& day);
    bool send_chest_state(const ChestState& chest);

    void shutdown();

private:
    BaseProtocol baseProtocol;

    bool send_game_msg(GameMsg msg);
    bool send_count(std::size_t count);
    // count u16 y, por cada slot, item u8, quantity u8 y quality u8.
    bool send_slots(const std::vector<InventorySlot>& slots);
    bool send_player_state(const PlayerState& player);
    bool send_entity_state(const EntityState& entity);
    bool send_appearance(const CharacterAppearance& appearance);
    std::optional<CharacterAppearance> recv_appearance();
};

#endif  // SERVER_PROTOCOL_H
