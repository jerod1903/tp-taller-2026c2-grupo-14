#ifndef MAPA_H
#define MAPA_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/sprites/sprite.h"
#include "common/map/MapDefinition.h"

struct TileGrafico {
    SDL2pp::Texture& textura;
    SDL2pp::Rect origen;
    std::optional<stardew::maps::TileType> debajo;
    bool espejado;
};

struct ElementoGrafico {
    SDL2pp::Texture& textura;
    SDL2pp::Rect origen;
    bool es_suelo;
    bool espejado;
};

class Mapa {
public:
    // mundo: el mapa del mundo (cargar_mundo); los interiores los arma el juego
    Mapa(SDL2pp::Renderer& renderer, const stardew::maps::MapDefinition& mundo, int max_jugadores);
    Mapa(const Mapa&) = delete;
    Mapa& operator=(const Mapa&) = delete;
    Mapa(Mapa&&) = delete;
    Mapa& operator=(Mapa&&) = delete;
    void mostrar_area(const std::string& area_id);
    void dibujar_suelo(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    void dibujar_tile(SDL2pp::Renderer& renderer, stardew::maps::TileType tipo, int columna,
                      int fila, const SDL2pp::Rect& camara);
    const std::vector<Sprite>& elementos() const;
    std::optional<Sprite> sprite_elemento(stardew::maps::ElementType tipo, int x, int y, int ancho,
                                          int alto) const;
    int ancho_px() const;
    int alto_px() const;

private:
    using Tiles = std::vector<std::vector<stardew::maps::TileType>>;
    struct Area {
        Tiles tiles;
        std::vector<Sprite> suelo;
        std::vector<Sprite> elementos;
    };
    static std::map<std::string, SDL2pp::Texture> cargar_hojas(SDL2pp::Renderer& renderer);
    std::map<stardew::maps::TileType, TileGrafico> armar_graficos();
    std::map<stardew::maps::ElementType, ElementoGrafico> armar_graficos_elementos();
    std::map<std::string, Area> cargar_areas(const stardew::maps::MapDefinition& mundo,
                                             int max_jugadores) const;
    void agregar_areas(stardew::maps::MapDefinition definicion,
                       std::map<std::string, Area>& cargadas) const;
    std::map<std::string, SDL2pp::Texture> hojas;
    std::map<stardew::maps::TileType, TileGrafico> graficos;
    std::map<stardew::maps::ElementType, ElementoGrafico> graficos_elementos;
    std::map<std::string, Area> areas;
    const Area* area_actual = nullptr;
};

#endif
