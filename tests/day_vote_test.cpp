#include "server/game/day_vote.h"

#include <gtest/gtest.h>

namespace {

TEST(DayVoteTest, EndsWhenEveryoneIsReady) {
    DayVote vote({1, 2});
    EXPECT_FALSE(vote.is_done());
    EXPECT_TRUE(vote.vote(1));
    EXPECT_FALSE(vote.is_done());
    EXPECT_EQ(vote.ready_count(), 1);
    EXPECT_EQ(vote.total(), 2);
    EXPECT_TRUE(vote.vote(2));
    EXPECT_TRUE(vote.is_done());
}

TEST(DayVoteTest, AnOkCountsOnceAndOnlyFromVoters) {
    DayVote vote({1, 2});
    EXPECT_TRUE(vote.vote(1));
    EXPECT_FALSE(vote.vote(1));
    EXPECT_FALSE(vote.vote(9));
    EXPECT_EQ(vote.ready_count(), 1);
}

TEST(DayVoteTest, WhoLeavesStopsCounting) {
    DayVote vote({1, 2});
    vote.vote(1);
    EXPECT_TRUE(vote.sync({1}));
    EXPECT_TRUE(vote.is_done());
    EXPECT_EQ(vote.total(), 1);
}

TEST(DayVoteTest, WhoEntersAlsoHasToVote) {
    DayVote vote({1});
    vote.vote(1);
    EXPECT_TRUE(vote.sync({1, 3}));
    EXPECT_FALSE(vote.is_done());
    EXPECT_EQ(vote.total(), 2);
    vote.vote(3);
    EXPECT_TRUE(vote.is_done());
}

TEST(DayVoteTest, SyncingTheSamePlayersChangesNothing) {
    DayVote vote({1, 2});
    vote.vote(2);
    EXPECT_FALSE(vote.sync({1, 2}));
    EXPECT_EQ(vote.ready_count(), 1);
}

}  // namespace
