#include "server/game/commerce/market.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "tests/world_helpers.h"

namespace {

constexpr int GOLD = 500;
constexpr int ENERGY = 100;
const Position MARNIE_POS{2, 2};
const Position PIERRE_POS{8, 2};
const Position CLINT_POS{2, 8};
const Position ANIMAL_SPAWN{1, 1};

std::vector<Merchant> merchants() {
    const ServerConfig::Market& prices = server_config().market;
    return {Merchant("Marnie", MARNIE_POS,
                     {{"vaca", AnimalKind::Cow, prices.cow_price},
                      {"gallina", AnimalKind::Chicken, prices.chicken_price}},
                     {{"vaca", AnimalKind::Cow, prices.cow_buyback}}),
            Merchant("Pierre", PIERRE_POS, {}, {{"leche", ItemType::Milk, prices.milk_buyback}}),
            Merchant("Clint", CLINT_POS, {}, {})};
}

class MarketTest: public ::testing::Test {
protected:
    TestLocation market_location{make_grid(12, 12)};
    TestLocation outside;
    Farm farm{"farm", make_grid(), Position{4, 4},
              std::make_unique<Cabin>("cabin", make_grid(), Position{0, 4})};
    Market market{market_location, merchants(), farm, ANIMAL_SPAWN};
    std::vector<std::unique_ptr<Player>> players;

    // Un jugador en `location`, en esa celda.
    Player& player_at(const Position& pos, Location& location) {
        players.push_back(std::make_unique<Player>(static_cast<uint32_t>(players.size() + 1), pos,
                                                   location, "jero", 100, 1.0f, GOLD, ENERGY));
        return *players.back();
    }

    // Un jugador en el mercado, una casilla debajo del comerciante, que ya le habló.
    Player& talking_to(const Position& merchant) {
        Player& player = player_at(Position{merchant.x, merchant.y + 1}, market_location);
        market.talk(player, merchant);
        return player;
    }
};

}  // namespace

TEST_F(MarketTest, ClickingAMerchantNearbyGreetsAndTires) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 1}, market_location);
    std::optional<ChatMessage> greeting = market.talk(player, MARNIE_POS);
    ASSERT_TRUE(greeting.has_value());
    EXPECT_EQ(greeting->kind, ChatKind::Merchant);
    EXPECT_EQ(greeting->from, "Marnie");
    EXPECT_EQ(player.get_energy(), ENERGY - server_config().market.talk_energy);
}

TEST_F(MarketTest, ClickingItsHeadAlsoCounts) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 1}, market_location);
    EXPECT_TRUE(market.talk(player, Position{MARNIE_POS.x, MARNIE_POS.y - 1}).has_value());
}

TEST_F(MarketTest, AMerchantTooFarAwayDoesNotAnswerNorTires) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 4}, market_location);
    EXPECT_FALSE(market.talk(player, MARNIE_POS).has_value());
    EXPECT_EQ(player.get_energy(), ENERGY);
}

TEST_F(MarketTest, FromAnotherAreaNobodyAnswers) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 1}, outside);
    EXPECT_FALSE(market.talk(player, MARNIE_POS).has_value());
}

TEST_F(MarketTest, ClickingAnEmptyCellDoesNothing) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 1}, market_location);
    EXPECT_FALSE(market.talk(player, Position{MARNIE_POS.x + 1, MARNIE_POS.y}).has_value());
}

TEST_F(MarketTest, WithoutTalkingFirstACommandGetsAWarning) {
    Player& player = player_at(Position{MARNIE_POS.x, MARNIE_POS.y + 1}, market_location);
    EXPECT_EQ(market.trade(player, "/consultar_catalogo").message.kind, ChatKind::System);
}

TEST_F(MarketTest, WalkingAwayEndsTheConversation) {
    Player& player = talking_to(MARNIE_POS);
    player.relocate(market_location, Position{MARNIE_POS.x, MARNIE_POS.y + 5});
    EXPECT_EQ(market.trade(player, "/consultar_catalogo").message.kind, ChatKind::System);
}

TEST_F(MarketTest, TheCatalogIsTheOneOfTheMerchantYouTalkTo) {
    TradeReply reply = market.trade(talking_to(MARNIE_POS), "/consultar_catalogo");
    EXPECT_EQ(reply.message.from, "Marnie");
    EXPECT_EQ(reply.message.text, market.find_merchant("Marnie")->catalog());
}

TEST_F(MarketTest, AnUnknownCommandRepeatsTheGreeting) {
    TradeReply reply = market.trade(talking_to(MARNIE_POS), "/bailar");
    EXPECT_EQ(reply.message.text, market.find_merchant("Marnie")->greeting());
}

TEST_F(MarketTest, BuyingAnimalsChargesAndPutsThemInTheFarm) {
    Player& player = talking_to(MARNIE_POS);
    TradeReply reply = market.trade(player, "/comprar vacas 2");
    EXPECT_EQ(player.get_gold(), GOLD - 2 * server_config().market.cow_price);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 2);
    EXPECT_FALSE(reply.inventory_changed);
}

TEST_F(MarketTest, WithoutAQuantityItBuysOne) {
    Player& player = talking_to(MARNIE_POS);
    market.trade(player, "/comprar gallina");
    EXPECT_EQ(farm.count_animals(AnimalKind::Chicken), 1);
}

TEST_F(MarketTest, WithoutEnoughGoldNothingIsBought) {
    Player& player = talking_to(MARNIE_POS);
    market.trade(player, "/comprar vaca 99");
    EXPECT_EQ(player.get_gold(), GOLD);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 0);
}

TEST_F(MarketTest, TheFarmLimitIsRespected) {
    Player& player = talking_to(MARNIE_POS);
    market.trade(player, "/comprar vaca " + std::to_string(server_config().market.max_cows + 1));
    EXPECT_EQ(player.get_gold(), GOLD);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 0);
}

TEST_F(MarketTest, SellingAnimalsPaysAndTakesThemFromTheFarm) {
    farm.add_animal(AnimalKind::Cow, ANIMAL_SPAWN);
    Player& player = talking_to(MARNIE_POS);
    market.trade(player, "/vender vaca");
    EXPECT_EQ(player.get_gold(), GOLD + server_config().market.cow_buyback);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 0);
}

TEST_F(MarketTest, PierreBuysMilkFromTheInventory) {
    Player& player = talking_to(PIERRE_POS);
    player.receive(ItemType::Milk, 3);
    TradeReply reply = market.trade(player, "/vender leche 3");
    EXPECT_TRUE(reply.inventory_changed);
    EXPECT_EQ(player.count(ItemType::Milk), 0);
    EXPECT_EQ(player.get_gold(), GOLD + 3 * server_config().market.milk_buyback);
}

TEST_F(MarketTest, SellingWhatYouDoNotHaveIsRejected) {
    Player& player = talking_to(PIERRE_POS);
    TradeReply reply = market.trade(player, "/vender leche 1");
    EXPECT_FALSE(reply.inventory_changed);
    EXPECT_EQ(player.get_gold(), GOLD);
}

TEST_F(MarketTest, WhatTheMerchantDoesNotSellIsRejected) {
    Player& player = talking_to(CLINT_POS);
    TradeReply reply = market.trade(player, "/comprar vaca");
    EXPECT_EQ(reply.message.from, "Clint");
    EXPECT_EQ(player.get_gold(), GOLD);
}

TEST_F(MarketTest, InvalidQuantitiesAreRejected) {
    Player& player = talking_to(MARNIE_POS);
    for (const char* command: {"/comprar vaca 0", "/comprar vaca abc", "/comprar vaca 100",
                               "/comprar vaca -1", "/comprar"}) {
        market.trade(player, command);
    }
    EXPECT_EQ(player.get_gold(), GOLD);
    EXPECT_EQ(farm.count_animals(AnimalKind::Cow), 0);
}
