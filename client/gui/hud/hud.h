#ifndef HUD_H
#define HUD_H

#include <cstdint>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "client/game/estado_juego.h"
#include "client/gui/chat/fuente.h"

// El HUD va a la mitad de la escala del mundo (redondeada para abajo, como mínimo 1) para no tapar
// tanto. La usan el HUD y el minimapa.
int escala_del_hud(int escala_mundo);

// La hora en formato HH:MM, redondeada para abajo a PASO_MINUTOS_RELOJ. Después de las 23:50
// sigue en 00:00.
std::string texto_hora(uint16_t minuto_del_dia);

// Arriba a la derecha, dos carteles colgados (el día y la hora con el reloj) y abajo el oro con la
// moneda; abajo a la derecha, las barras de vida y de energía. Se dibuja sobre la ventana: los
// assets a una escala entera y el texto con una fuente propia, un poco más grande que la del chat.
class Hud {
public:
    explicit Hud(SDL2pp::Renderer& renderer);
    // escala: la del mundo (el HUD va a escala_del_hud).
    // No dibuja nada hasta el primer snapshot.
    void dibujar(SDL2pp::Renderer& renderer, const EstadoJuego& estado, int escala);

private:
    void dibujar_carteles(SDL2pp::Renderer& renderer, uint16_t dia, uint16_t minuto_del_dia,
                          const SDL2pp::Point& esquina, int escala);
    void dibujar_oro(SDL2pp::Renderer& renderer, uint32_t oro, const SDL2pp::Point& esquina,
                     int escala);
    // Una barra vertical que se llena desde abajo, con la etiqueta arriba
    void dibujar_barra(SDL2pp::Renderer& renderer, uint16_t valor, int maximo, const char* etiqueta,
                       const SDL2pp::Point& esquina, int escala);
    // En el cartel que empieza en `cartel`: el icono a la izquierda y el texto alineado a la
    // derecha
    void escribir_con_icono(SDL2pp::Renderer& renderer, SDL2pp::Texture& icono,
                            const std::string& texto, const SDL2pp::Point& cartel, int escala);
    // Con una sombra abajo a la derecha. `x` es el borde izquierdo o, si `a_la_derecha`, el
    // derecho; el texto se centra en vertical en `centro_y`.
    void escribir(SDL2pp::Renderer& renderer, const std::string& texto, int x, int centro_y,
                  bool a_la_derecha);
    void escribir_centrado(SDL2pp::Renderer& renderer, const std::string& texto,
                           const SDL2pp::Rect& caja);

    Fuente fuente;
    SDL2pp::Texture carteles;
    SDL2pp::Texture marco;
    SDL2pp::Texture icono_reloj;
    SDL2pp::Texture icono_oro;
};

#endif
