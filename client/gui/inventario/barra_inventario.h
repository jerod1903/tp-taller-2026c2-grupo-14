#ifndef BARRA_INVENTARIO_H
#define BARRA_INVENTARIO_H

#include <map>

#include <SDL2pp/SDL2pp.hh>

#include "common/game_dto.h"

class BarraInventario {
public:
    explicit BarraInventario(SDL2pp::Renderer& renderer);
    void dibujar(SDL2pp::Renderer& renderer, const InventoryState& inventario,
                 const SDL2pp::Point& tamano_lienzo);

private:
    void dibujar_casillero(SDL2pp::Renderer& renderer, const InventorySlot& slot,
                           const SDL2pp::Rect& caja, bool seleccionado);
    void dibujar_icono(SDL2pp::Renderer& renderer, const InventorySlot& slot,
                       const SDL2pp::Rect& caja);
    void dibujar_numero(SDL2pp::Renderer& renderer, int numero, const SDL2pp::Point& esquina);
    std::map<ItemType, SDL2pp::Texture> iconos;
    SDL2pp::Texture hoja_cultivos;
};

#endif
