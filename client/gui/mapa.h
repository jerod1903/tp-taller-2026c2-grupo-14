#ifndef MAPA_H
#define MAPA_H

#include <map>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/map/MapDefinition.h"

#include "sprite.h"

// Con qué imagen se dibuja un tipo de tile: la textura y la casilla (columna, fila) dentro de ella
struct TileGrafico {
    SDL2pp::Texture& textura;
    SDL2pp::Point casilla;
};

// Con qué imagen se dibuja un elemento del mapa (cabaña, NPC, ...). Se escala al ancho del
// elemento y se apoya en su borde de abajo. Los de suelo se pisan: van debajo de los personajes
struct ElementoGrafico {
    SDL2pp::Texture& textura;
    SDL2pp::Rect origen;
    bool es_suelo;
};

// Dibuja el mapa base (los tiles y los elementos fijos) del mundo que indica el servidor. Lo que
// cambia durante la partida (cultivos, jugadores, etc.) lo manda el servidor. Todo en pixeles del
// tileset: la Interfaz escala el resultado a la ventana
class Mapa {
public:
    Mapa(SDL2pp::Renderer& renderer, const std::string& world_id);
    // graficos guarda referencias a los tilesets de este mismo objeto: no se copia ni se mueve
    Mapa(const Mapa&) = delete;
    Mapa& operator=(const Mapa&) = delete;
    Mapa(Mapa&&) = delete;
    Mapa& operator=(Mapa&&) = delete;
    // Elige el area a dibujar. Si el mundo no la tiene (p. ej. el interior de la cabaña, que lo
    // arma el servidor) no se dibuja nada
    void mostrar_area(const std::string& area_id);
    // Los tiles y los elementos de suelo que entran en la camara
    void dibujar_suelo(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    // Las estructuras y NPCs del area: la Interfaz los dibuja junto con los personajes
    const std::vector<Sprite>& elementos() const;
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
    // Que textura y casilla usa cada tipo de tile. Se llama con los tilesets ya cargados
    std::map<stardew::maps::TileType, TileGrafico> armar_graficos();
    // Que textura y recorte usa cada tipo de elemento
    std::map<stardew::maps::ElementType, ElementoGrafico> armar_graficos_elementos();
    // Carga maps/<world_id>.toml y devuelve los tiles y elementos de cada area, por id
    std::map<std::string, Area> cargar_areas(const std::string& world_id) const;
    SDL2pp::Texture tileset_caverna;
    SDL2pp::Texture tileset_exterior;
    SDL2pp::Texture cabanas;
    SDL2pp::Texture edificios_pueblo;
    SDL2pp::Texture npc_marnie;
    SDL2pp::Texture npc_clint;
    SDL2pp::Texture npc_pierre;
    SDL2pp::Texture npc_robin;
    std::map<stardew::maps::TileType, TileGrafico> graficos;
    std::map<stardew::maps::ElementType, ElementoGrafico> graficos_elementos;
    std::map<std::string, Area> areas;
    // nullptr hasta que se elija un area que exista
    const Area* area_actual = nullptr;
};

#endif
