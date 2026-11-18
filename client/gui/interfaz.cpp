#include "interfaz.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <SDL_image.h>

#include "client/config/client_config.h"
#include "client/constants.h"
#include "client/gui/rutas.h"
#include "common/map/CabinInterior.h"

Interfaz::Interfaz():
        sdl(SDL_INIT_VIDEO),
        sdl_image(IMG_INIT_PNG),
        ventana("Jueguito", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                client_config().ventana.ancho, client_config().ventana.alto,
                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        renderer(ventana, -1,
                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE),
        constructor_capas(renderer),
        hoja_vaca(renderer, RUTA_HOJA_VACA),
        hoja_gallina(renderer, RUTA_HOJA_GALLINA),
        aspectos{{EntityType::Player, {&FORMATO_PERSONAJE, {}}},
                 {EntityType::Cow, {&FORMATO_VACA, {capa_animal(hoja_vaca, FORMATO_VACA)}}},
                 {EntityType::Chicken,
                  {&FORMATO_GALLINA, {capa_animal(hoja_gallina, FORMATO_GALLINA)}}}},
        barra(renderer),
        fuente(RUTA_FUENTE, TAMANO_FUENTE_CHAT),
        fuente_titulo(RUTA_FUENTE, TAMANO_FUENTE_TITULO),
        panel_chat(fuente),
        menu_pausa(renderer, fuente, fuente_titulo),
        hud(renderer),
        globo(renderer),
        transicion(DURACION_TRANSICION) {
    for (const auto& [item, ruta]: RUTAS_ICONOS) {
        objetos_piso.try_emplace(item, renderer, ruta);
    }
}

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
        if (menu_pausa.abierto()) {
            leer_pausa(evento, entrada);
            continue;
        }
        if (borrador) {
            leer_chat(evento, entrada);
            continue;
        }
        if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT &&
            camara) {
            entrada.click = casilla_clickeada(evento.button.x, evento.button.y);
            continue;
        }
        if (evento.type != SDL_KEYDOWN || evento.key.repeat) {
            continue;
        }
        SDL_Keycode tecla = evento.key.keysym.sym;
        if (tecla == SDLK_ESCAPE) {
            menu_pausa.abrir();
        } else if (tecla == SDLK_RETURN) {
            borrador = "";
            entrada.escribiendo = true;
        } else if (tecla == SDLK_e) {
            entrada.interactuar = true;
        } else if (tecla == SDLK_x) {
            entrada.cruzar_puerta = true;
        } else if (tecla == SDLK_F11) {
            alternar_pantalla_completa();
        } else if (tecla >= SDLK_1 && tecla <= SDLK_9) {
            entrada.slot_seleccionado = static_cast<uint8_t>(tecla - SDLK_1);
        }
    }
    if (borrador || menu_pausa.abierto()) {
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

void Interfaz::leer_pausa(const SDL_Event& evento, Entrada& entrada) {
    std::optional<OpcionPausa> opcion = menu_pausa.leer(evento, renderer.GetOutputSize());
    if (opcion == OpcionPausa::Salir) {
        entrada.salir = true;
    } else if (opcion == OpcionPausa::Invitar) {
        entrada.invitar = true;
    }
}

ClickRequest Interfaz::casilla_clickeada(int x, int y) const {
    int x_mundo = x / escala + camara->x;
    int y_mundo = y / escala + camara->y;
    return ClickRequest{static_cast<uint16_t>(x_mundo / TAMANO_TILE),
                        static_cast<uint16_t>(y_mundo / TAMANO_TILE)};
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
        entrada.escribiendo = false;
    } else if (tecla == SDLK_ESCAPE) {
        borrador.reset();
        entrada.escribiendo = false;
    }
}

void Interfaz::alternar_pantalla_completa() {
    pantalla_completa = !pantalla_completa;
    ventana.SetFullscreen(pantalla_completa ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void Interfaz::actualizar_mapa(const EstadoJuego& estado) {
    if (estado.mundo && (!mapa || *estado.mundo != mundo_del_mapa)) {
        mapa.emplace(renderer, *estado.mundo, estado.max_jugadores);
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
        it->second.actualizar(entidad, estado.tick, estado.duracion_tick, ahora);
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
    for (const auto& [item, sonido]: SONIDOS_AL_RECIBIR) {
        int cantidad = 0;
        for (const InventorySlot& slot: estado.inventario.slots) {
            cantidad += slot.item == item ? slot.quantity : 0;
        }
        auto anterior = cantidades_anteriores.find(item);
        if (anterior != cantidades_anteriores.end() && cantidad > anterior->second) {
            reproductor.reproducir(sonido);
        }
        cantidades_anteriores[item] = cantidad;
    }
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
        proximo_paso = ahora + client_config().audio.intervalo_paso;
    }
}

void Interfaz::ajustar_lienzo(int ancho_area, int alto_area) {
    SDL2pp::Point salida = renderer.GetOutputSize();
    escala = std::max(1, salida.y / client_config().ventana.alto_mundo_visible);
    if (ancho_area > 0 && alto_area > 0) {
        escala = std::max({escala, (salida.x + ancho_area - 1) / ancho_area,
                           (salida.y + alto_area - 1) / alto_area});
    }
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
    camas.clear();
    std::copy_if(estado.entidades.begin(), estado.entidades.end(), std::back_inserter(camas),
                 [](const EntityState& entidad) { return entidad.type == EntityType::Bed; });
    actualizar_sonidos(estado, ahora);
    auto propio = entidades.find({EntityType::Player, estado.id_propio});
    if (propio != entidades.end() && propio->second.area() != area_anterior) {
        if (!area_anterior.empty()) {
            congelar_lienzo(ahora);
        }
        area_anterior = propio->second.area();
    }
    bool hay_area = mapa && propio != entidades.end();
    if (hay_area) {
        mapa->mostrar_area(propio->second.area());
    }
    ajustar_lienzo(hay_area ? mapa->ancho_px() : 0, hay_area ? mapa->alto_px() : 0);
    renderer.SetTarget(*lienzo);
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();
    if (propio != entidades.end()) {
        if (transicion.oscureciendo(ahora) && congelado) {
            renderer.Copy(*congelado);
        } else {
            dibujar_area(propio->second, estado.granja);
        }
        oscurecer(transicion.opacidad(ahora));
        dibujar_inventario(estado.inventario);
    }
    renderer.SetTarget();
    renderer.Clear();
    renderer.Copy(*lienzo, SDL2pp::NullOpt,
                  SDL2pp::Rect(0, 0, lienzo->GetWidth() * escala, lienzo->GetHeight() * escala));
    panel_chat.dibujar(renderer, estado.chat, borrador, renderer.GetOutputSize(), ahora);
    hud.dibujar(renderer, estado, escala);
    if (menu_pausa.abierto()) {
        menu_pausa.dibujar(renderer, renderer.GetOutputSize());
    }
    renderer.Present();
}

void Interfaz::dibujar_inventario(const InventoryState& inventario) {
    barra.dibujar(renderer, inventario, SDL2pp::Point(lienzo->GetWidth(), lienzo->GetHeight()));
}

void Interfaz::congelar_lienzo(std::chrono::steady_clock::time_point ahora) {
    if (!lienzo) {
        return;
    }
    congelado.emplace(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
                      lienzo->GetWidth(), lienzo->GetHeight());
    renderer.SetTarget(*congelado);
    renderer.Copy(*lienzo);
    renderer.SetTarget();
    transicion.empezar(ahora);
}

void Interfaz::oscurecer(uint8_t opacidad) {
    if (opacidad == 0) {
        return;
    }
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(0, 0, 0, opacidad);
    renderer.FillRect(SDL2pp::Rect(0, 0, lienzo->GetWidth(), lienzo->GetHeight()));
}

std::optional<Sprite> Interfaz::sprite_objeto(const GroundItem& objeto) {
    auto textura = objetos_piso.find(objeto.item);
    if (textura == objetos_piso.end()) {
        return std::nullopt;
    }
    SDL2pp::Point tamano = textura->second.GetSize();
    SDL2pp::Point esquina(objeto.x * TAMANO_TILE + (TAMANO_TILE - tamano.x) / 2,
                          objeto.y * TAMANO_TILE + (TAMANO_TILE - tamano.y) / 2);
    return Sprite{&textura->second, SDL2pp::Rect(SDL2pp::Point(0, 0), tamano),
                  SDL2pp::Rect(esquina, tamano)};
}

void Interfaz::dibujar_area(const EntidadGrafica& propio, const FarmState& granja) {
    const std::string& area = propio.area();
    int ancho_mapa = mapa ? mapa->ancho_px() : 0;
    int alto_mapa = mapa ? mapa->alto_px() : 0;
    SDL2pp::Rect camara = calculador_camara.calcular(propio.centro(), ancho_mapa, alto_mapa);
    this->camara = camara;
    std::vector<Sprite> sprites;
    if (mapa) {
        mapa->dibujar_suelo(renderer, camara);
        sprites = mapa->elementos();
    }
    for (const EntityState& cama: camas) {
        if (!mapa || cama.areaId != area) {
            continue;
        }
        namespace maps = stardew::maps;
        int x = static_cast<int>(cama.x / POSITION_SCALE);
        int y = static_cast<int>(cama.y / POSITION_SCALE);
        int alto_frazada = maps::BED_HEIGHT - maps::BED_HEAD_HEIGHT;
        for (std::optional<Sprite> parte:
             {mapa->sprite_elemento(maps::ElementType::Bed, x, y, maps::BED_WIDTH,
                                    maps::BED_HEAD_HEIGHT),
              mapa->sprite_elemento(maps::ElementType::BedBlanket, x, y + maps::BED_HEAD_HEIGHT,
                                    maps::BED_WIDTH, alto_frazada)}) {
            if (parte) {
                sprites.push_back(*parte);
            }
        }
    }
    if (granja.areaId == area) {
        for (const GroundItem& objeto: granja.items) {
            if (std::optional<Sprite> sprite = sprite_objeto(objeto)) {
                sprites.push_back(*sprite);
            }
        }
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
    for (const auto& [clave, entidad]: entidades) {
        if (entidad.area() == area && entidad.emote() != Emote::None) {
            globo.dibujar(renderer, entidad.emote(), entidad.cabeza(), camara);
        }
    }
}
