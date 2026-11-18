#include "server/net/server_protocol.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/net/socket.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18643";

// Cada mensaje entra en el buffer del socket: no hace falta un thread del otro lado.
class ServerProtocolTest: public ::testing::Test {
protected:
    void SetUp() override {
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server = std::make_unique<ServerProtocol>(listener.accept());
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
    }

    void expect_msg(GameMsg expected) {
        std::optional<uint8_t> tag = client->recv_u8();
        ASSERT_TRUE(tag.has_value());
        EXPECT_EQ(static_cast<GameMsg>(*tag), expected);
    }

    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
};

TEST_F(ServerProtocolTest, LocationDataGoesWithItsTagWorldIdTickRateMaxPlayersAndMaxStats) {
    ASSERT_TRUE(server->send_location_data(LocationData{"granja", 30, 4, 270, 100}));
    expect_msg(GameMsg::LocationData);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(30));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(4));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(270));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(100));
}

TEST_F(ServerProtocolTest, SnapshotGoesWithTickAndTheStateOfEachPlayer) {
    PlayerState player{3, "granja", 14250, 6500, -1, 0, 90, 80, 500};
    ASSERT_TRUE(server->send_snapshot(Snapshot{42, 3, 125, {player}, {}}));
    expect_msg(GameMsg::Snapshot);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(42));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(3));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(125));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(1));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(3));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(14250));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(6500));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(-1)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(90));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(80));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(500));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));  // sin entidades
}

TEST_F(ServerProtocolTest, SnapshotEntitiesGoAfterThePlayers) {
    EntityState cow{2,  EntityType::Cow,     "granja",          3500, 4500, 1,
                    -1, Emote::CowMilkReady, EntityAction::Hurt};
    ASSERT_TRUE(server->send_snapshot(Snapshot{7, 1, 0, {}, {cow}}));
    expect_msg(GameMsg::Snapshot);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(7));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(1));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(1));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(2));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(EntityType::Cow)));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(3500));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(4500));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(1));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(-1)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(Emote::CowMilkReady)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(EntityAction::Hurt)));
}

TEST_F(ServerProtocolTest, PlayerInfoGoesWithIdNameAndAppearance) {
    const PlayerInfo info{3, "Martu", CharacterAppearance{5, 2, 7, 1, 4}};
    ASSERT_TRUE(server->send_player_info(info));
    expect_msg(GameMsg::PlayerInfo);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(3));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("Martu"));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(5));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(2));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(7));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(1));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(4));
}

TEST_F(ServerProtocolTest, GameEndedGoesWithItsTagAndReason) {
    ASSERT_TRUE(server->send_game_ended(GameEnded{EndReason::Left}));
    expect_msg(GameMsg::GameEnded);
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(EndReason::Left)));
}

TEST_F(ServerProtocolTest, EmptySnapshotHasNoPlayers) {
    ASSERT_TRUE(server->send_snapshot(Snapshot{0, 1, 0, {}, {}}));
    expect_msg(GameMsg::Snapshot);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(1));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
}

TEST_F(ServerProtocolTest, RecvGameOpReadsMove) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::Move)));
    EXPECT_EQ(server->recv_game_op(), std::optional<GameOp>(GameOp::Move));
}

TEST_F(ServerProtocolTest, RecvGameOpReadsLeaveGame) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::LeaveGame)));
    EXPECT_EQ(server->recv_game_op(), std::optional<GameOp>(GameOp::LeaveGame));
}

TEST_F(ServerProtocolTest, RecvGameOpClosedWithoutDataIsNullopt) {
    client->shutdown();
    EXPECT_FALSE(server->recv_game_op().has_value());
}

// El protocolo no valida el tag: detectar una op desconocida es trabajo de la CommandFactory.
TEST_F(ServerProtocolTest, RecvGameOpDoesNotValidateUnknownTag) {
    ASSERT_TRUE(client->send_u8(0xFF));
    EXPECT_EQ(server->recv_game_op(), std::optional<GameOp>(static_cast<GameOp>(0xFF)));
}

TEST_F(ServerProtocolTest, RecvMoveReadsDirections) {
    ASSERT_TRUE(client->send_u8(1));
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(-1)));
    std::optional<MoveRequest> move = server->recv_move();
    ASSERT_TRUE(move.has_value());
    EXPECT_EQ(move->dirX, 1);
    EXPECT_EQ(move->dirY, -1);
}

TEST_F(ServerProtocolTest, RecvMoveReadsStop) {
    ASSERT_TRUE(client->send_u8(0));
    ASSERT_TRUE(client->send_u8(0));
    std::optional<MoveRequest> move = server->recv_move();
    ASSERT_TRUE(move.has_value());
    EXPECT_EQ(move->dirX, 0);
    EXPECT_EQ(move->dirY, 0);
}

TEST_F(ServerProtocolTest, RecvMoveCutInTheMiddleIsNullopt) {
    ASSERT_TRUE(client->send_u8(1));
    client->shutdown();
    EXPECT_FALSE(server->recv_move().has_value());
}

TEST_F(ServerProtocolTest, RecvMoveClosedWithoutDataIsNullopt) {
    client->shutdown();
    EXPECT_FALSE(server->recv_move().has_value());
}

// Si recv_move leyera de más o de menos, el mensaje siguiente quedaría desalineado.
TEST_F(ServerProtocolTest, RecvMoveConsumesExactlyItsPayload) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::Move)));
    ASSERT_TRUE(client->send_u8(1));
    ASSERT_TRUE(client->send_u8(0));
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::LeaveGame)));

    EXPECT_EQ(server->recv_game_op(), std::optional<GameOp>(GameOp::Move));
    std::optional<MoveRequest> move = server->recv_move();
    ASSERT_TRUE(move.has_value());
    EXPECT_EQ(move->dirX, 1);
    EXPECT_EQ(move->dirY, 0);
    EXPECT_EQ(server->recv_game_op(), std::optional<GameOp>(GameOp::LeaveGame));
}


}  // namespace

TEST_F(ServerProtocolTest, RecvChatReadsTheText) {
    ASSERT_TRUE(client->send_string("hola, gente!"));
    std::optional<ChatRequest> chat = server->recv_chat();
    ASSERT_TRUE(chat.has_value());
    EXPECT_EQ(chat->text, "hola, gente!");
}

TEST_F(ServerProtocolTest, RecvChatClosedWithoutDataIsNullopt) {
    client->shutdown();
    EXPECT_FALSE(server->recv_chat().has_value());
}

TEST_F(ServerProtocolTest, ChatMessageGoesWithItsTagKindAuthorAndText) {
    ASSERT_TRUE(server->send_chat_message(ChatMessage{ChatKind::Private, "Ana", "vamos?"}));
    EXPECT_EQ(client->recv_u8(), static_cast<uint8_t>(GameMsg::ChatMessage));
    EXPECT_EQ(client->recv_u8(), static_cast<uint8_t>(ChatKind::Private));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("Ana"));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("vamos?"));
}

TEST_F(ServerProtocolTest, RecvTypingReadsOpenAndClosed) {
    ASSERT_TRUE(client->send_u8(1));
    ASSERT_TRUE(client->send_u8(0));
    EXPECT_EQ(server->recv_typing(), std::optional<TypingRequest>(TypingRequest{true}));
    EXPECT_EQ(server->recv_typing(), std::optional<TypingRequest>(TypingRequest{false}));
}

TEST_F(ServerProtocolTest, RecvTypingClosedWithoutDataIsNullopt) {
    client->shutdown();
    EXPECT_FALSE(server->recv_typing().has_value());
}

TEST_F(ServerProtocolTest, RecvClickReadsTheCell) {
    ASSERT_TRUE(client->send_u16(14));
    ASSERT_TRUE(client->send_u16(3));
    EXPECT_EQ(server->recv_click(), std::optional<ClickRequest>(ClickRequest{14, 3}));
}

TEST_F(ServerProtocolTest, RecvClickCutInTheMiddleIsNullopt) {
    ASSERT_TRUE(client->send_u16(14));
    client->shutdown();
    EXPECT_FALSE(server->recv_click().has_value());
}

TEST_F(ServerProtocolTest, FarmStateGoesWithTagAreaAndEachTile) {
    const FarmState farm{"granja",
                         {{10, 5, SoilType::Dirt, CropType::None, 0, false, false},
                          {11, 5, SoilType::Tilled, CropType::Strawberry, 2, true, false}}};
    ASSERT_TRUE(server->send_farm_state(farm));
    expect_msg(GameMsg::FarmState);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(2));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(10));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(5));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(SoilType::Dirt)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(CropType::None)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(0));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(0));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(11));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(5));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(SoilType::Tilled)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(CropType::Strawberry)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(2));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(1));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(0));
}

TEST_F(ServerProtocolTest, EmptyFarmStateHasNoTiles) {
    ASSERT_TRUE(server->send_farm_state(FarmState{"granja", {}}));
    expect_msg(GameMsg::FarmState);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
}

TEST_F(ServerProtocolTest, InventoryStateGoesWithSelectedSlotAndEachSlot) {
    const InventoryState inventory{1, {{ItemType::Shovel, 1}, {ItemType::Potato, 4}}};
    ASSERT_TRUE(server->send_inventory_state(inventory));
    expect_msg(GameMsg::InventoryState);
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(1));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(2));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(ItemType::Shovel)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(1));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(ItemQuality::Base)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(ItemType::Potato)));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(4));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(ItemQuality::Base)));
}
