#include "interfaz.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL_image.h>

#include "client/constants.h"
#include "rutas.h"

Interfaz::Interfaz():
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana("Jueguito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ANCHO_VENTANA,
                ALTO_VENTANA, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        renderer(ventana, -1,
                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                         SDL_RENDERER_TARGETTEXTURE),
        sprite_personaje(renderer, RUTA_SPRITE_PERSONAJE) {}

Entrada Interfaz::leer_entrada() {
    Entrada entrada;
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT ||
            (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE)) {
            entrada.salir = true;
        }
        if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_F11 &&
            !evento.key.repeat) {
            alternar_pantalla_completa();
        }
    }
    static std::map<std::pair<int, int>, std::pair<SDL_Scancode, SDL_Scancode>> teclas_direccion = {
            {{-1, 0}, {SDL_SCANCODE_LEFT, SDL_SCANCODE_A}},
            {{1, 0}, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D}},
            {{0, -1}, {SDL_SCANCODE_UP, SDL_SCANCODE_W}},
            {{0, 1}, {SDL_SCANCODE_DOWN, SDL_SCANCODE_S}}};
    const Uint8* teclas = SDL_GetKeyboardState(nullptr);
    int dir_x = 0;
    int dir_y = 0;
    for (const auto& [direccion, par_teclas]: teclas_direccion) {
        if (teclas[par_teclas.first] || teclas[par_teclas.second]) {
            dir_x += direccion.first;
            dir_y += direccion.second;
        }
    }
    entrada.dir_x = static_cast<int8_t>(dir_x);
    entrada.dir_y = static_cast<int8_t>(dir_y);
    return entrada;
}

void Interfaz::alternar_pantalla_completa() {
    pantalla_completa = !pantalla_completa;
    ventana.SetFullscreen(pantalla_completa ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void Interfaz::actualizar_mapa(const EstadoJuego& estado) {
    if (estado.mundo && (!mapa || *estado.mundo != mundo_del_mapa)) {
        mapa.emplace(renderer, *estado.mundo);
        mundo_del_mapa = *estado.mundo;
    }
}

void Interfaz::actualizar_personajes(const EstadoJuego& estado,
                                     std::chrono::steady_clock::time_point ahora) {
    std::set<uint32_t> presentes;
    for (const PlayerState& jugador: estado.jugadores) {
        presentes.insert(jugador.playerId);
        auto it = personajes.try_emplace(jugador.playerId, sprite_personaje).first;
        it->second.actualizar(jugador, estado.tick, ahora);
    }
    std::erase_if(personajes,
                  [&presentes](const auto& par) { return !presentes.contains(par.first); });
}

void Interfaz::actualizar_sonidos(const EstadoJuego& estado,
                                  std::chrono::steady_clock::time_point ahora) {
    auto propio = std::find_if(
            estado.jugadores.begin(), estado.jugadores.end(),
            [&estado](const PlayerState& jugador) { return jugador.playerId == estado.id_propio; });
    if (propio == estado.jugadores.end()) {
        return;
    }
    // Hay varios frames por Snapshot: se compara solo cuando llega uno nuevo
    if (estado.tick != tick_escuchado) {
        if (propio_anterior) {
            bool tiene_direccion = propio->dirX != 0 || propio->dirY != 0;
            bool se_movio = propio->x != propio_anterior->x || propio->y != propio_anterior->y;
            caminando = tiene_direccion && se_movio;
            if (propio->gold > propio_anterior->gold) {
                reproductor.reproducir(Sonido::Moneda);
            }
        }
        propio_anterior = *propio;
        tick_escuchado = estado.tick;
    }
    if (caminando && ahora >= proximo_paso) {
        reproductor.reproducir(SONIDOS_PASO[paso_actual]);
        paso_actual = (paso_actual + 1) % SONIDOS_PASO.size();
        proximo_paso = ahora + INTERVALO_PASO;
    }
}

void Interfaz::ajustar_lienzo() {
    SDL2pp::Point salida = renderer.GetOutputSize();
    // Escala entera, para que todos los pixeles midan lo mismo, y elegida para que se vea mas o
    // menos la misma porcion del mundo en cualquier tamaño de ventana
    escala = std::max(1, salida.y / ALTO_MUNDO_VISIBLE);
    int ancho = std::max(1, (salida.x + escala - 1) / escala);
    int alto = std::max(1, (salida.y + escala - 1) / escala);
    if (!lienzo || lienzo->GetWidth() != ancho || lienzo->GetHeight() != alto) {
        // ARGB8888 es un formato nativo del renderer; con RGBA8888 cada frame tardaba un 60% mas
        lienzo.emplace(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, ancho, alto);
        calculador_camara.ajustar_vista(ancho, alto);
    }
}

void Interfaz::dibujar(const EstadoJuego& estado) {
    auto ahora = std::chrono::steady_clock::now();
    actualizar_mapa(estado);
    actualizar_personajes(estado, ahora);
    actualizar_sonidos(estado, ahora);
    ajustar_lienzo();
    renderer.SetTarget(*lienzo);
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    // Sin personaje propio no se sabe que area mostrar
    auto propio = personajes.find(estado.id_propio);
    if (propio != personajes.end()) {
        dibujar_area(propio->second);
    }
    renderer.SetTarget();
    renderer.Clear();
    renderer.Copy(*lienzo, SDL2pp::NullOpt,
                  SDL2pp::Rect(0, 0, lienzo->GetWidth() * escala, lienzo->GetHeight() * escala));
    renderer.Present();
}

void Interfaz::dibujar_area(const Personaje& propio) {
    const std::string& area = propio.area();
    if (mapa) {
        mapa->mostrar_area(area);
    }
    int ancho_mapa = mapa ? mapa->ancho_px() : 0;
    int alto_mapa = mapa ? mapa->alto_px() : 0;
    SDL2pp::Rect camara = calculador_camara.calcular(propio.centro(), ancho_mapa, alto_mapa);
    std::vector<Sprite> sprites;
    if (mapa) {
        mapa->dibujar_suelo(renderer, camara);
        sprites = mapa->elementos();
    }
    // Solo los que estan en la misma area que este jugador
    for (const auto& [id, personaje]: personajes) {
        if (personaje.area() == area) {
            sprites.push_back(personaje.sprite());
        }
    }
    // Lo que apoya mas abajo esta mas adelante: se dibuja despues y tapa a lo de atras
    std::stable_sort(sprites.begin(), sprites.end(),
                     [](const Sprite& a, const Sprite& b) { return a.base() < b.base(); });
    for (const Sprite& sprite: sprites) {
        sprite.dibujar(renderer, camara);
    }
}
