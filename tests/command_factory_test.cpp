#include "server/game/commands/command_factory.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/net/socket.h"
#include "server/net/server_protocol.h"
#include "tests/fake_game_context.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18646";
constexpr uint32_t PLAYER_ID = FakeGameContext::PLAYER_ID;

// El payload entra en el buffer del socket: no hace falta un thread del otro lado.
// Cada test mira el efecto del comando sobre un FakeGameContext, no su tipo.
class CommandFactoryTest: public ::testing::Test {
protected:
    void SetUp() override {
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server = std::make_unique<ServerProtocol>(listener.accept());
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
        factory = std::make_unique<CommandFactory>(*server);
    }

    // Solo el payload: la op ya la leyó el Receiver antes de llamar a create.
    void send_move_payload(int8_t dir_x, int8_t dir_y) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_x)));
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_y)));
    }

    void send_select_slot_payload(uint8_t slot) { ASSERT_TRUE(client->send_u8(slot)); }

    FakeGameContext context;
    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<CommandFactory> factory;
};

TEST_F(CommandFactoryTest, MoveReadsItsPayloadAndBuildsAMoveCommand) {
    send_move_payload(1, -1);
    std::shared_ptr<GameCommand> command = factory->create(GameOp::Move, PLAYER_ID);
    ASSERT_NE(command, nullptr);
    command->execute(context);
    EXPECT_EQ(context.player.get_direction(), Direction::UP_RIGHT);
}

TEST_F(CommandFactoryTest, TheCommandIsForThePlayerItWasCreatedFor) {
    send_move_payload(1, 0);
    std::shared_ptr<GameCommand> command = factory->create(GameOp::Move, PLAYER_ID + 1);
    ASSERT_NE(command, nullptr);
    command->execute(context);
    EXPECT_EQ(context.player.get_direction(), Direction::NONE);
}

TEST_F(CommandFactoryTest, ConsecutiveMovesReadOnlyTheirOwnPayload) {
    send_move_payload(1, 0);
    send_move_payload(0, 1);
    std::shared_ptr<GameCommand> first = factory->create(GameOp::Move, PLAYER_ID);
    std::shared_ptr<GameCommand> second = factory->create(GameOp::Move, PLAYER_ID);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    first->execute(context);
    EXPECT_EQ(context.player.get_direction(), Direction::RIGHT);
    second->execute(context);
    EXPECT_EQ(context.player.get_direction(), Direction::DOWN);
}

TEST_F(CommandFactoryTest, MoveCutInTheMiddleOfThePayloadIsNullptr) {
    ASSERT_TRUE(client->send_u8(1));
    client->shutdown();
    EXPECT_EQ(factory->create(GameOp::Move, PLAYER_ID), nullptr);
}

TEST_F(CommandFactoryTest, UnknownOpIsNullptr) {
    EXPECT_EQ(factory->create(static_cast<GameOp>(0xFF), PLAYER_ID), nullptr);
}

// La resuelve el Receiver: si la armara la factory, el Receiver no sabría que el jugador salió.
TEST_F(CommandFactoryTest, LeaveGameIsNotRegistered) {
    EXPECT_EQ(factory->create(GameOp::LeaveGame, PLAYER_ID), nullptr);
}

TEST_F(CommandFactoryTest, SelectSlotReadsItsPayloadAndBuildsASelectSlotCommand) {
    send_select_slot_payload(5);
    std::shared_ptr<GameCommand> command = factory->create(GameOp::SelectSlot, PLAYER_ID);
    ASSERT_NE(command, nullptr);

    command->execute(context);

    // Verificamos que el comando impactó en el jugador (el slot 5)
    EXPECT_EQ(context.player.get_inventory_state().selectedSlot, 5);
}

TEST_F(CommandFactoryTest, SelectSlotClosedWithoutDataIsNullptr) {
    client->shutdown();
    EXPECT_EQ(factory->create(GameOp::SelectSlot, PLAYER_ID), nullptr);
}


}  // namespace
