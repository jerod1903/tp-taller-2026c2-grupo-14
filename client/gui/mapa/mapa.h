#ifndef MAPA_H
#define MAPA_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/sprites/sprite.h"
#include "common/map/MapDefinition.h"

// Con qué imagen se dibuja un tipo de tile: la textura, el recorte (de una casilla de ancho, a
// veces menos de alto) y el tile que va debajo si tiene partes transparentes
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
    // max_jugadores: el ala este de la cabaña tiene una cama por jugador
    Mapa(SDL2pp::Renderer& renderer, const std::string& world_id, int max_jugadores);
    // graficos guarda referencias a las hojas de este mismo objeto: no se copia ni se mueve
    Mapa(const Mapa&) = delete;
    Mapa& operator=(const Mapa&) = delete;
    Mapa(Mapa&&) = delete;
    Mapa& operator=(Mapa&&) = delete;
    // Elige el area a dibujar. Si no existe no se dibuja nada
    void mostrar_area(const std::string& area_id);
    void dibujar_suelo(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    // Un tile en la casilla (columna, fila), con lo que va debajo
    void dibujar_tile(SDL2pp::Renderer& renderer, stardew::maps::TileType tipo, int columna,
                      int fila, const SDL2pp::Rect& camara);
    // Las estructuras y NPCs del area: la Interfaz los dibuja junto con los personajes
    const std::vector<Sprite>& elementos() const;
    // Un elemento que no esta en el mapa (como una cama), con su esquina de arriba a la izquierda
    // en la casilla (x, y). Vacio si el tipo no tiene dibujo
    std::optional<Sprite> sprite_elemento(stardew::maps::ElementType tipo, int x, int y, int ancho,
                                          int alto) const;
    // Del area que se esta mostrando
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
    // Que textura y recorte usa cada tipo de tile, segun TILE_SPRITES. Con las hojas ya cargadas
    std::map<stardew::maps::TileType, TileGrafico> armar_graficos();
    std::map<stardew::maps::ElementType, ElementoGrafico> armar_graficos_elementos();
    // Carga maps/<world_id>.toml y el interior de la cabaña, y devuelve los tiles y elementos de
    // cada area, por id
    std::map<std::string, Area> cargar_areas(const std::string& world_id, int max_jugadores) const;
    void agregar_areas(stardew::maps::MapDefinition definicion,
                       std::map<std::string, Area>& cargadas) const;
    // Ruta de la tabla (relativa a Assets/) -> su imagen
    std::map<std::string, SDL2pp::Texture> hojas;
    std::map<stardew::maps::TileType, TileGrafico> graficos;
    std::map<stardew::maps::ElementType, ElementoGrafico> graficos_elementos;
    std::map<std::string, Area> areas;
    const Area* area_actual = nullptr;
};

#endif
