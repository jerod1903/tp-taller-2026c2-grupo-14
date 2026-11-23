#include "server/game/commerce/merchant.h"

#include <string>
#include <variant>

#include <gtest/gtest.h>

namespace {

const Position POS{16, 8};

Merchant marnie() {
    return Merchant("Marnie", POS,
                    {{"vaca", AnimalKind::Cow, 80}, {"gallina", AnimalKind::Chicken, 20}},
                    {{"vaca", AnimalKind::Cow, 40}});
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

}  // namespace

TEST(MerchantTest, OccupiesItsCellAndTheOneAbove) {
    Merchant merchant = marnie();
    EXPECT_TRUE(merchant.occupies(POS));
    EXPECT_TRUE(merchant.occupies(Position{POS.x, POS.y - 1}));
}

TEST(MerchantTest, DoesNotOccupyTheCellsAroundIt) {
    Merchant merchant = marnie();
    EXPECT_FALSE(merchant.occupies(Position{POS.x, POS.y + 1}));
    EXPECT_FALSE(merchant.occupies(Position{POS.x + 1, POS.y}));
    EXPECT_FALSE(merchant.occupies(Position{POS.x, POS.y - 2}));
}

TEST(MerchantTest, FindsWhatItSellsByName) {
    Merchant merchant = marnie();
    const Offer* offer = merchant.find_sold("gallina");
    ASSERT_NE(offer, nullptr);
    EXPECT_EQ(offer->price, 20);
    EXPECT_EQ(std::get<AnimalKind>(offer->goods), AnimalKind::Chicken);
}

TEST(MerchantTest, ThePluralFindsTheSameOffer) {
    Merchant merchant = marnie();
    EXPECT_EQ(merchant.find_sold("vacas"), merchant.find_sold("vaca"));
}

TEST(MerchantTest, WhatItSellsAndWhatItBuysAreSeparate) {
    Merchant merchant = marnie();
    EXPECT_EQ(merchant.find_sold("vaca")->price, 80);
    EXPECT_EQ(merchant.find_bought("vaca")->price, 40);
    EXPECT_EQ(merchant.find_bought("gallina"), nullptr);
}

TEST(MerchantTest, AnUnknownProductIsNotFound) {
    EXPECT_EQ(marnie().find_sold("orbe_oro"), nullptr);
    EXPECT_EQ(marnie().find_sold(""), nullptr);
}

TEST(MerchantTest, TheGreetingSaysItsNameAndTheCommands) {
    std::string greeting = marnie().greeting();
    EXPECT_TRUE(contains(greeting, "Marnie"));
    EXPECT_TRUE(contains(greeting, "/consultar_catalogo"));
    EXPECT_TRUE(contains(greeting, "/comprar"));
    EXPECT_TRUE(contains(greeting, "/vender"));
}

TEST(MerchantTest, TheCatalogListsNamesAndPrices) {
    EXPECT_EQ(marnie().catalog(), "Vendo: vaca (80), gallina (20). Compro: vaca (40).");
}

TEST(MerchantTest, AMerchantWithNothingSaysSo) {
    Merchant clint("Clint", POS, {}, {});
    EXPECT_EQ(clint.catalog(), "Vendo: nada por ahora. Compro: nada por ahora.");
}
