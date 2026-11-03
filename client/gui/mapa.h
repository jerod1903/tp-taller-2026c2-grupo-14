#ifndef MAPA_H
#define MAPA_H

#include <map>
#include <string>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "common/map/MapDefinition.h"

// Con qué imagen se dibuja un tipo de tile: la textura y la casilla (columna, fila) dentro de ella
struct TileGrafico {
    SDL2pp::Texture& textura;
    SDL2pp::Point casilla;
};

// Dibuja el mapa base (los tiles) del mundo que indica el servidor. Lo que cambia durante la
// partida (cultivos, jugadores, etc.) lo manda el servidor 
class Mapa {
public:
    Mapa(SDL2pp::Renderer& renderer, const std::string& world_id, int escala);
    // graficos guarda referencias a los tilesets de este mismo objeto: no se copia ni se mueve
    Mapa(const Mapa&) = delete;
    Mapa& operator=(const Mapa&) = delete;
    Mapa(Mapa&&) = delete;
    Mapa& operator=(Mapa&&) = delete;
    // Elige el area a dibujar. Si el mundo no la tiene (p. ej. el interior de la cabaña, que lo
    // arma el servidor) no se dibuja nada
    void mostrar_area(const std::string& area_id);
    void dibujar(SDL2pp::Renderer& renderer, const SDL2pp::Rect& camara);
    // Del area que se esta mostrando
    int ancho_px() const;
    int alto_px() const;

private:
    using Tiles = std::vector<std::vector<stardew::maps::TileType>>;
    // Que textura y casilla usa cada tipo de tile. Se llama con los tilesets ya cargados
    std::map<stardew::maps::TileType, TileGrafico> armar_graficos();
    // Carga maps/<world_id>.toml y devuelve los tiles de cada area, por id
    static std::map<std::string, Tiles> cargar_areas(const std::string& world_id);
    SDL2pp::Texture tileset_caverna;
    SDL2pp::Texture tileset_exterior;
    std::map<stardew::maps::TileType, TileGrafico> graficos;
    std::map<std::string, Tiles> areas;
    // nullptr hasta que se elija un area que exista
    const Tiles* area_actual = nullptr;
    int escala;
};

#endif
