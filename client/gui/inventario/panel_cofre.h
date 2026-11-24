#ifndef PANEL_COFRE_H
#define PANEL_COFRE_H

#include <optional>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "client/gui/inventario/barra_inventario.h"
#include "common/game_dto.h"

// El cofre abierto: su grilla y, abajo, el inventario del jugador, centrados en el lienzo.
class PanelCofre {
public:
    PanelCofre(SDL2pp::Renderer& renderer, BarraInventario& barra);
    void dibujar(SDL2pp::Renderer& renderer, const std::vector<InventorySlot>& cofre,
                 const InventoryState& inventario, const SDL2pp::Point& tamano_lienzo);
    // El casillero bajo ese punto del lienzo, con el layout del último dibujar; nullopt si no hay.
    std::optional<ChestSlotRequest> casillero_en(const SDL2pp::Point& punto) const;

private:
    // Dónde va ese casillero, según el layout del último dibujar
    SDL2pp::Rect caja(ChestSide lado, int indice) const;
    BarraInventario& barra;
    SDL2pp::Texture marco;
    SDL2pp::Rect contorno;
    int cantidad_cofre = 0;
    int cantidad_jugador = 0;
};

#endif
