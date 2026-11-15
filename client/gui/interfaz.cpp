#include "interfaz.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL_image.h>

#include "client/constants.h"
#include "client/gui/rutas.h"

Interfaz::Interfaz():
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana("Jueguito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ANCHO_VENTANA,
                ALTO_VENTANA, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        renderer(ventana, -1,
                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                         SDL_RENDERER_TARGETTEXTURE),
        constructor_capas(renderer),
        hoja_vaca(renderer, RUTA_HOJA_VACA),
        hoja_gallina(renderer, RUTA_HOJA_GALLINA),
        aspectos{{EntityType::Player, {&FORMATO_PERSONAJE, {}}},
                 {EntityType::Cow, {&FORMATO_VACA, {capa_animal(hoja_vaca, FORMATO_VACA)}}},
                 {EntityType::Chicken,
                  {&FORMATO_GALLINA, {capa_animal(hoja_gallina, FORMATO_GALLINA)}}}},
        barra(renderer),
        fuente(RUTA_FUENTE, TAMANO_FUENTE_CHAT),
        panel_chat(fuente) {}

Capa Interfaz::capa_animal(SDL2pp::Texture& hoja, const FormatoAnimacion& formato) {
    SDL2pp::Point cuadro(formato.ancho_cuadro, formato.alto_cuadro);
    return Capa{&hoja, TipoCapa::PorCuadro, SDL2pp::Point(0, 0), cuadro, SDL2pp::Point(0, 0)};
}

namespace {
void borrar_ultima_letra(std::string& texto) {
    while (!texto.empty() && (static_cast<unsigned char>(texto.back()) & 0xC0) == 0x80) {
        texto.pop_back();
    }
    if (!texto.empty()) {
        texto.pop_back();
    }
}
}

Entrada Interfaz::leer_entrada() {
    Entrada entrada;
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if (evento.type == SDL_QUIT) {
            entrada.salir = true;
        }
        if (borrador) {
            leer_chat(evento, entrada);
            continue;
        }
        if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
            continue;
        }
        SDL_Keycode tecla = evento.key.keysym.sym;
        if (tecla == SDLK_ESCAPE) {
            entrada.salir = true;
        } else if (tecla == SDLK_RETURN) {
            borrador = "";
        } else if (tecla == SDLK_e) {
            entrada.interactuar = true;
        } else if (tecla == SDLK_F11) {
            alternar_pantalla_completa();
        } else if (tecla >= SDLK_1 && tecla <= SDLK_9) {
            entrada.slot_seleccionado = static_cast<uint8_t>(tecla - SDLK_1);
        }
    }
    if (borrador) {
        return entrada;
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

void Interfaz::leer_chat(const SDL_Event& evento, Entrada& entrada) {
    if (evento.type == SDL_TEXTINPUT &&
        borrador->size() + std::strlen(evento.text.text) <= MAX_CHAT_LENGTH) {
        borrador->append(evento.text.text);
    }
    if (evento.type != SDL_KEYDOWN) {
        return;
    }
    SDL_Keycode tecla = evento.key.keysym.sym;
    if (tecla == SDLK_BACKSPACE) {
        borrar_ultima_letra(*borrador);
    } else if (tecla == SDLK_RETURN) {
        if (!borrador->empty()) {
            entrada.mensaje_chat = *borrador;
        }
        borrador.reset();
    } else if (tecla == SDLK_ESCAPE) {
        borrador.reset();
    }
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

void Interfaz::actualizar_entidades(const EstadoJuego& estado,
                                    std::chrono::steady_clock::time_point ahora) {
    std::set<std::pair<EntityType, uint32_t>> presentes;
    for (const EntityState& entidad: estado.entidades) {
        auto aspecto = aspectos.find(entidad.type);
        if (aspecto == aspectos.end()) {
            continue;
        }
        std::pair<EntityType, uint32_t> clave(entidad.type, entidad.entityId);
        presentes.insert(clave);
        auto [it, nueva] =
                entidades.try_emplace(clave, *aspecto->second.formato, aspecto->second.capas);
        if (entidad.type == EntityType::Player) {
            vestir(it->second, entidad.entityId, estado, nueva);
        }
        it->second.actualizar(entidad, estado.tick, ahora);
    }
    for (auto it = entidades.begin(); it != entidades.end();) {
        if (presentes.contains(it->first)) {
            ++it;
            continue;
        }
        if (it->first.first == EntityType::Player) {
            vestidos.erase(it->first.second);
            constructor_capas.olvidar(it->first.second);
        }
        it = entidades.erase(it);
    }
}

void Interfaz::vestir(EntidadGrafica& jugador, uint32_t player_id, const EstadoJuego& estado,
                      bool recien_creado) {
    auto info = estado.info_jugadores.find(player_id);
    bool tiene_apariencia = info != estado.info_jugadores.end();
    if (!recien_creado && (!tiene_apariencia || vestidos.contains(player_id))) {
        return;
    }
    std::optional<CharacterAppearance> elegida;
    if (tiene_apariencia) {
        elegida = info->second.appearance;
        vestidos.insert(player_id);
    }
    jugador.vestir(constructor_capas.capas(player_id, elegida));
}

void Interfaz::actualizar_sonidos(const EstadoJuego& estado,
                                  std::chrono::steady_clock::time_point ahora) {
    int leche = 0;
    for (const InventorySlot& slot: estado.inventario.slots) {
        leche += slot.item == ItemType::Milk ? slot.quantity : 0;
    }
    if (leche_anterior && leche > *leche_anterior) {
        reproductor.reproducir(Sonido::Ordenar);
    }
    leche_anterior = leche;
    auto propio = std::find_if(
            estado.jugadores.begin(), estado.jugadores.end(),
            [&estado](const PlayerState& jugador) { return jugador.playerId == estado.id_propio; });
    if (propio == estado.jugadores.end()) {
        return;
    }
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
        reproductor.reproducir(Sonido::Paso);
        proximo_paso = ahora + INTERVALO_PASO;
    }
}

void Interfaz::ajustar_lienzo() {
    SDL2pp::Point salida = renderer.GetOutputSize();
    escala = std::max(1, salida.y / ALTO_MUNDO_VISIBLE);
    int ancho = std::max(1, (salida.x + escala - 1) / escala);
    int alto = std::max(1, (salida.y + escala - 1) / escala);
    if (!lienzo || lienzo->GetWidth() != ancho || lienzo->GetHeight() != alto) {
        lienzo.emplace(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, ancho, alto);
        calculador_camara.ajustar_vista(ancho, alto);
    }
}

void Interfaz::dibujar(const EstadoJuego& estado) {
    auto ahora = std::chrono::steady_clock::now();
    actualizar_mapa(estado);
    actualizar_entidades(estado, ahora);
    actualizar_sonidos(estado, ahora);
    ajustar_lienzo();
    renderer.SetTarget(*lienzo);
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    auto propio = entidades.find({EntityType::Player, estado.id_propio});
    if (propio != entidades.end()) {
        dibujar_area(propio->second);
        dibujar_inventario(estado.inventario);
    }
    renderer.SetTarget();
    renderer.Clear();
    renderer.Copy(*lienzo, SDL2pp::NullOpt,
                  SDL2pp::Rect(0, 0, lienzo->GetWidth() * escala, lienzo->GetHeight() * escala));
    panel_chat.dibujar(renderer, estado.chat, borrador, renderer.GetOutputSize(), ahora);
    renderer.Present();
}

void Interfaz::dibujar_inventario(const InventoryState& inventario) {
    barra.dibujar(renderer, inventario, SDL2pp::Point(lienzo->GetWidth(), lienzo->GetHeight()));
}

void Interfaz::dibujar_area(const EntidadGrafica& propio) {
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
    for (const auto& [clave, entidad]: entidades) {
        if (entidad.area() == area) {
            std::vector<Sprite> capas = entidad.sprites();
            sprites.insert(sprites.end(), capas.begin(), capas.end());
        }
    }
    std::stable_sort(sprites.begin(), sprites.end(),
                     [](const Sprite& a, const Sprite& b) { return a.base() < b.base(); });
    for (const Sprite& sprite: sprites) {
        sprite.dibujar(renderer, camara);
    }
}
