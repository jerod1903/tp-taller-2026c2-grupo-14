#ifndef CONSTRUCTOR_CAPAS_H
#define CONSTRUCTOR_CAPAS_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/lobby_dto.h"

#include "capa.h"

// Arma las capas con las que se dibuja a un jugador segun su apariencia (la que eligio en el menu y
// manda el servidor en su PlayerInfo): cuerpo, pantalon, camisa, brazos, pelo y sombrero, en el
// orden en que se tapan. Carga las hojas del granjero una sola vez
class ConstructorCapas {
public:
    explicit ConstructorCapas(SDL2pp::Renderer& renderer);
    // Las hojas son de este objeto: las capas apuntan a ellas
    ConstructorCapas(const ConstructorCapas&) = delete;
    ConstructorCapas& operator=(const ConstructorCapas&) = delete;
    // Sin apariencia (todavia no llego su PlayerInfo): solo el cuerpo y los brazos
    std::vector<Capa> capas(uint32_t player_id,
                            const std::optional<CharacterAppearance>& apariencia);
    // Libera lo que se armo para un jugador que se fue
    void olvidar(uint32_t player_id);

private:
    // Los brazos con las mangas del color de su camisa. Es la unica capa que no se comparte: se
    // arma una vez por jugador
    SDL2pp::Texture& brazos_de(uint32_t player_id, uint16_t camisa);
    SDL2pp::Renderer& renderer;
    // Para leer los pixeles de los brazos y de las camisas
    SDL2pp::Surface pixeles_cuerpo;
    SDL2pp::Surface pixeles_camisas;
    SDL2pp::Texture cuerpo;
    SDL2pp::Texture pelo;
    SDL2pp::Texture camisas;
    SDL2pp::Texture pantalones;
    SDL2pp::Texture sombreros;
    // player_id -> sus brazos
    std::map<uint32_t, SDL2pp::Texture> brazos;
};

#endif
