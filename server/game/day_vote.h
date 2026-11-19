#ifndef DAY_VOTE_H
#define DAY_VOTE_H

#include <cstdint>
#include <set>

// La votación para pasar al día siguiente: votan los jugadores conectados, y termina cuando todos
// dieron el Ok. El que entra mientras se vota también tiene que darlo; el que se va deja de
// contar. Un Ok no se retira.
class DayVote {
public:
    // `voters`: los conectados cuando terminó el día.
    explicit DayVote(std::set<uint32_t> voters);

    // Suma a los que entraron y saca a los que se fueron. true si cambió algo.
    bool sync(const std::set<uint32_t>& connected);
    // true si contaba y todavía no lo había dado.
    bool vote(uint32_t player_id);
    // Ya votaron todos los que cuentan.
    bool is_done() const;
    uint8_t ready_count() const;
    uint8_t total() const;

private:
    std::set<uint32_t> waiting;
    std::set<uint32_t> ready;
};

#endif  // DAY_VOTE_H
