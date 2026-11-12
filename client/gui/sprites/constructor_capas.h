#ifndef CONSTRUCTOR_CAPAS_H
#define CONSTRUCTOR_CAPAS_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/lobby_dto.h"

#include "client/gui/sprites/capa.h"

class ConstructorCapas {
public:
    explicit ConstructorCapas(SDL2pp::Renderer& renderer);
    ConstructorCapas(const ConstructorCapas&) = delete;
    ConstructorCapas& operator=(const ConstructorCapas&) = delete;
    std::vector<Capa> capas(uint32_t player_id,
                            const std::optional<CharacterAppearance>& apariencia);
    void olvidar(uint32_t player_id);

private:
    SDL2pp::Texture& brazos_de(uint32_t player_id, uint16_t camisa);
    SDL2pp::Renderer& renderer;
    SDL2pp::Surface pixeles_cuerpo;
    SDL2pp::Surface pixeles_camisas;
    SDL2pp::Texture cuerpo;
    SDL2pp::Texture pelo;
    SDL2pp::Texture camisas;
    SDL2pp::Texture pantalones;
    SDL2pp::Texture sombreros;
    std::map<uint32_t, SDL2pp::Texture> brazos;
};

#endif
