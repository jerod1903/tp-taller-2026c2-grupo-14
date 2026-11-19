#include "server/game/world/game_clock.h"

namespace {
constexpr float MINUTES_PER_DAY = 1200.0f;  // de 6:00 a 2:00
}  // namespace

GameClock::GameClock(float day_seconds): day_seconds(day_seconds) {}

uint16_t GameClock::get_day() const { return day; }

float GameClock::get_elapsed() const { return elapsed; }

float GameClock::get_minute_of_day() const { return elapsed / day_seconds * MINUTES_PER_DAY; }

float GameClock::get_day_seconds() const { return day_seconds; }

bool GameClock::update(float dt) {
    elapsed += dt;
    if (elapsed >= day_seconds) {
        elapsed = day_seconds;
        return true;
    }
    return false;
}

void GameClock::end_day() { elapsed = day_seconds; }

void GameClock::next_day() {
    day++;
    elapsed = 0.0f;
}

void GameClock::start_day(uint16_t day) {
    this->day = day;
    elapsed = 0.0f;
}
