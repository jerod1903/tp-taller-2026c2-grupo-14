#ifndef BARRA_INVENTARIO_H
#define BARRA_INVENTARIO_H

#include <map>
#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/inventario/casillero.h"
#include "client/gui/inventario/objeto.h"

class BarraInventario {
public:
    explicit BarraInventario(SDL2pp::Renderer& renderer);
    void dibujar(SDL2pp::Renderer& renderer, const std::vector<std::optional<Casillero>>& casilleros,
                 int seleccionado, const SDL2pp::Point& tamano_lienzo);

private:
    struct Icono {
        SDL2pp::Texture textura;
        SDL2pp::Rect recorte;
    };
    void dibujar_casillero(SDL2pp::Renderer& renderer, const std::optional<Casillero>& casillero,
                           const SDL2pp::Rect& caja, bool seleccionado);
    void dibujar_icono(SDL2pp::Renderer& renderer, Objeto objeto, const SDL2pp::Rect& caja);
    void dibujar_numero(SDL2pp::Renderer& renderer, int numero, const SDL2pp::Point& esquina);
    std::map<Objeto, Icono> iconos;
};

#endif
