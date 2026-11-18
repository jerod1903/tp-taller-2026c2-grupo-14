#include "client/gui/hud/hud.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "client/constants.h"
#include "client/gui/rutas.h"

namespace {
constexpr int MINUTOS_POR_HORA = 60;
constexpr int HORAS_POR_DIA = 24;

const PlayerState* buscar_propio(const EstadoJuego& estado) {
    auto propio = std::find_if(
            estado.jugadores.begin(), estado.jugadores.end(),
            [&estado](const PlayerState& jugador) { return jugador.playerId == estado.id_propio; });
    return propio == estado.jugadores.end() ? nullptr : &*propio;
}

const SDL_Color& color_energia(int porcentaje) {
    if (porcentaje <= PORCENTAJE_ENERGIA_BAJA) {
        return COLOR_ENERGIA_BAJA;
    }
    return porcentaje <= PORCENTAJE_ENERGIA_MEDIA ? COLOR_ENERGIA_MEDIA : COLOR_ENERGIA_ALTA;
}

// El icono centrado en vertical en la cara del cartel que empieza en `y`
SDL2pp::Rect en_cartel(SDL2pp::Texture& icono, int x, int y, int escala) {
    SDL2pp::Point tamano = icono.GetSize() * escala;
    return SDL2pp::Rect(x, y + (ALTO_CARA_CARTEL * escala - tamano.y) / 2, tamano.x, tamano.y);
}
}  // namespace

std::string texto_hora(uint16_t minuto_del_dia) {
    int minutos = minuto_del_dia / PASO_MINUTOS_RELOJ * PASO_MINUTOS_RELOJ;
    int hora = (HORA_INICIO_DIA + minutos / MINUTOS_POR_HORA) % HORAS_POR_DIA;
    char texto[] = "00:00";
    std::snprintf(texto, sizeof(texto), "%02d:%02d", hora, minutos % MINUTOS_POR_HORA);
    return texto;
}

Hud::Hud(SDL2pp::Renderer& renderer):
        fuente(RUTA_FUENTE, TAMANO_FUENTE_HUD),
        carteles(renderer, RUTA_CARTELES_HUD),
        marco(renderer, RUTA_MARCO_HUD),
        icono_reloj(renderer, RUTA_ICONO_RELOJ),
        icono_oro(renderer, RUTA_ICONO_ORO) {}

void Hud::dibujar(SDL2pp::Renderer& renderer, const EstadoJuego& estado, int escala) {
    const PlayerState* propio = buscar_propio(estado);
    if (estado.dia == 0 || propio == nullptr || estado.energia_maxima <= 0) {
        return;
    }
    int escala_hud = std::max(1, escala / 2);
    SDL2pp::Point ventana = renderer.GetOutputSize();
    int margen = MARGEN_HUD * escala_hud;
    SDL2pp::Point carteles_en(ventana.x - margen - ANCHO_CARTEL * escala_hud, margen);
    dibujar_carteles(renderer, estado.dia, estado.minuto_del_dia, carteles_en, escala_hud);
    int alto_carteles = (PASO_CARTELES * (CANTIDAD_CARTELES - 1) + ALTO_CARTEL) * escala_hud;
    dibujar_oro(renderer, propio->gold,
                SDL2pp::Point(carteles_en.x,
                              carteles_en.y + alto_carteles + SEPARACION_CARTEL_ORO * escala_hud),
                escala_hud);
    dibujar_energia(renderer, propio->energy, estado.energia_maxima,
                    SDL2pp::Point(ventana.x - margen - ANCHO_BARRA_ENERGIA * escala_hud,
                                  ventana.y - margen - ALTO_BARRA_ENERGIA * escala_hud),
                    escala_hud);
}

void Hud::dibujar_carteles(SDL2pp::Renderer& renderer, uint16_t dia, uint16_t minuto_del_dia,
                           const SDL2pp::Point& esquina, int escala) {
    int alto = PASO_CARTELES * (CANTIDAD_CARTELES - 1) + ALTO_CARTEL;
    renderer.Copy(carteles, SDL2pp::Rect(0, 0, ANCHO_CARTEL, alto),
                  SDL2pp::Rect(esquina.x, esquina.y, ANCHO_CARTEL * escala, alto * escala));
    escribir_centrado(renderer, PREFIJO_DIA + std::to_string(dia),
                      SDL2pp::Rect(esquina.x, esquina.y, ANCHO_CARTEL * escala,
                                   ALTO_CARA_CARTEL * escala));
    SDL2pp::Point cartel_hora(esquina.x, esquina.y + PASO_CARTELES * escala);
    escribir_con_icono(renderer, icono_reloj, texto_hora(minuto_del_dia), cartel_hora, escala);
}

void Hud::dibujar_oro(SDL2pp::Renderer& renderer, uint32_t oro, const SDL2pp::Point& esquina,
                      int escala) {
    renderer.Copy(carteles, SDL2pp::Rect(0, 0, ANCHO_CARTEL, ALTO_CARTEL),
                  SDL2pp::Rect(esquina.x, esquina.y, ANCHO_CARTEL * escala, ALTO_CARTEL * escala));
    escribir_con_icono(renderer, icono_oro, std::to_string(oro), esquina, escala);
}

void Hud::escribir_con_icono(SDL2pp::Renderer& renderer, SDL2pp::Texture& icono,
                             const std::string& texto, const SDL2pp::Point& cartel, int escala) {
    renderer.Copy(icono, SDL2pp::NullOpt,
                  en_cartel(icono, cartel.x + RELLENO_CARTEL * escala, cartel.y, escala));
    escribir(renderer, texto, cartel.x + (ANCHO_CARTEL - RELLENO_CARTEL) * escala,
             cartel.y + ALTO_CARA_CARTEL * escala / 2, true);
}

void Hud::dibujar_energia(SDL2pp::Renderer& renderer, uint16_t energia, int energia_maxima,
                          const SDL2pp::Point& esquina, int escala) {
    SDL2pp::Rect caja(esquina.x, esquina.y, ANCHO_BARRA_ENERGIA * escala,
                      ALTO_BARRA_ENERGIA * escala);
    dibujar_marco(renderer, caja, escala);
    int borde = BORDE_MARCO * escala;
    SDL2pp::Rect interior(caja.x + borde, caja.y + borde, caja.w - 2 * borde, caja.h - 2 * borde);
    int porcentaje = std::clamp(energia * 100 / energia_maxima, 0, 100);
    int alto = interior.h * porcentaje / 100;
    const SDL_Color& color = color_energia(porcentaje);
    renderer.SetDrawColor(color.r, color.g, color.b, color.a);
    renderer.FillRect(
            SDL2pp::Rect(interior.x, interior.y + interior.h - alto, interior.w, alto));
    escribir_centrado(renderer, ETIQUETA_ENERGIA,
                      SDL2pp::Rect(caja.x, caja.y - fuente.alto_linea(), caja.w,
                                   fuente.alto_linea()));
}

void Hud::dibujar_marco(SDL2pp::Renderer& renderer, const SDL2pp::Rect& destino, int escala) {
    SDL2pp::Point origen = marco.GetSize();
    int borde = BORDE_MARCO;
    int borde_destino = BORDE_MARCO * escala;
    // Por eje: el borde de un lado, el medio que se estira y el borde del otro
    const int origen_x[] = {0, borde, origen.x - borde};
    const int origen_w[] = {borde, origen.x - 2 * borde, borde};
    const int origen_y[] = {0, borde, origen.y - borde};
    const int origen_h[] = {borde, origen.y - 2 * borde, borde};
    const int destino_x[] = {destino.x, destino.x + borde_destino,
                             destino.x + destino.w - borde_destino};
    const int destino_w[] = {borde_destino, destino.w - 2 * borde_destino, borde_destino};
    const int destino_y[] = {destino.y, destino.y + borde_destino,
                             destino.y + destino.h - borde_destino};
    const int destino_h[] = {borde_destino, destino.h - 2 * borde_destino, borde_destino};
    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            renderer.Copy(marco,
                          SDL2pp::Rect(origen_x[columna], origen_y[fila], origen_w[columna],
                                       origen_h[fila]),
                          SDL2pp::Rect(destino_x[columna], destino_y[fila], destino_w[columna],
                                       destino_h[fila]));
        }
    }
}

void Hud::escribir(SDL2pp::Renderer& renderer, const std::string& texto, int x, int centro_y,
                   bool a_la_derecha) {
    SDL2pp::Texture sombra = fuente.renderizar(renderer, texto, COLOR_SOMBRA_HUD);
    SDL2pp::Texture frente = fuente.renderizar(renderer, texto, COLOR_TEXTO_HUD);
    SDL2pp::Point tamano = frente.GetSize();
    SDL2pp::Point esquina(a_la_derecha ? x - tamano.x : x, centro_y - tamano.y / 2);
    renderer.Copy(sombra, SDL2pp::NullOpt, SDL2pp::Rect(esquina + SDL2pp::Point(1, 1), tamano));
    renderer.Copy(frente, SDL2pp::NullOpt, SDL2pp::Rect(esquina, tamano));
}

void Hud::escribir_centrado(SDL2pp::Renderer& renderer, const std::string& texto,
                            const SDL2pp::Rect& caja) {
    escribir(renderer, texto, caja.x + (caja.w + fuente.ancho(texto)) / 2, caja.y + caja.h / 2,
             true);
}
