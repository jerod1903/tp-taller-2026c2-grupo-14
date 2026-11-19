#ifndef MARCO_H
#define MARCO_H

#include <SDL2pp/SDL2pp.hh>

// El marco de madera del HUD (RUTA_MARCO_HUD) estirado a `destino` sin deformar el borde: las
// esquinas y los bordes quedan de BORDE_MARCO * escala y el medio se estira. Lo usan las barras de
// vida y energía y el minimapa.
void dibujar_marco(SDL2pp::Renderer& renderer, SDL2pp::Texture& marco, const SDL2pp::Rect& destino,
                   int escala);

#endif
