#include "client/gui/sprites/constructor_capas.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "client/constants.h"
#include "client/menu/styles/CharacterColors.h"
#include "common/character/CharacterSheets.h"

#include "client/gui/rutas.h"

namespace hojas = stardew::character;

namespace {

std::string ruta(const char* hoja) { return std::string(CARPETA_ASSETS) + hoja; }

SDL2pp::Point punto(hojas::Point p) { return SDL2pp::Point(p.x, p.y); }

SDL_Color color(uint32_t rgb) {
    return SDL_Color{static_cast<Uint8>(rgb >> 16), static_cast<Uint8>(rgb >> 8),
                     static_cast<Uint8>(rgb), 255};
}

SDL_Color color_de_pelo(uint8_t indice) {
    const char* hex = CharacterColors::PALETTE[indice % CharacterColors::PALETTE.size()];
    return color(std::stoul(hex + 1, nullptr, BASE_HEXADECIMAL));
}

Capa por_cuadro(SDL2pp::Texture& hoja, hojas::Point celda, SDL_Color tinte = SIN_TINTE) {
    SDL2pp::Point sin_desplazar(0, 0);
    return Capa{&hoja, TipoCapa::PorCuadro, punto(celda), punto(hojas::FRAME_SIZE), sin_desplazar,
                {}, tinte};
}

Capa por_vista(SDL2pp::Texture& hoja, hojas::Point celda, hojas::Point tamano,
               hojas::Point desplazamiento, const Vistas& vistas, SDL_Color tinte = SIN_TINTE) {
    return Capa{&hoja,  TipoCapa::PorVista, punto(celda), punto(tamano), punto(desplazamiento),
                vistas, tinte};
}

uint32_t& pixel(SDL2pp::Surface& superficie, int x, int y) {
    auto* fila = static_cast<uint8_t*>(superficie.Get()->pixels) + y * superficie.Get()->pitch;
    return reinterpret_cast<uint32_t*>(fila)[x];
}

std::map<uint32_t, uint32_t> colores_de_manga(SDL2pp::Surface& camisas, uint16_t camisa) {
    hojas::Point celda = hojas::shirt_cell(camisa);
    std::vector<uint32_t> frente;
    for (int y = 0; y < hojas::SHIRT_SIZE.y; ++y) {
        for (int x = 0; x < hojas::SHIRT_SIZE.x; ++x) {
            frente.push_back(pixel(camisas, celda.x + x, celda.y + y));
        }
    }
    std::vector<uint32_t> colores = hojas::sleeve_colors_of(frente);
    std::map<uint32_t, uint32_t> reemplazos;
    for (std::size_t i = 0; i < hojas::SLEEVE_COLORS.size() && !colores.empty(); ++i) {
        reemplazos[hojas::SLEEVE_COLORS[i]] = colores[std::min(i, colores.size() - 1)];
    }
    return reemplazos;
}

}

ConstructorCapas::ConstructorCapas(SDL2pp::Renderer& renderer):
        renderer(renderer),
        pixeles_cuerpo(SDL2pp::Surface(ruta(hojas::SHEET_BODY)).Convert(SDL_PIXELFORMAT_ARGB8888)),
        pixeles_camisas(
                SDL2pp::Surface(ruta(hojas::SHEET_SHIRTS)).Convert(SDL_PIXELFORMAT_ARGB8888)),
        cuerpo(renderer, pixeles_cuerpo),
        pelo(renderer, ruta(hojas::SHEET_HAIR)),
        camisas(renderer, pixeles_camisas),
        pantalones(renderer, ruta(hojas::SHEET_PANTS)),
        sombreros(renderer, ruta(hojas::SHEET_HATS)) {}

std::vector<Capa> ConstructorCapas::capas(uint32_t player_id,
                                             const std::optional<CharacterAppearance>& apariencia) {
    const hojas::Point brazos_en_hoja{hojas::ARMS_COLUMNS * hojas::FRAME_SIZE.x, 0};
    if (!apariencia) {
        return {por_cuadro(cuerpo, {0, 0}), por_cuadro(cuerpo, brazos_en_hoja)};
    }
    const CharacterAppearance& elegida = *apariencia;
    std::vector<Capa> resultado = {
            por_cuadro(cuerpo, {0, 0}),
            por_cuadro(pantalones, hojas::pants_block(elegida.pants), color(hojas::PANTS_COLOR)),
            por_vista(camisas, hojas::shirt_cell(elegida.shirt), hojas::SHIRT_SIZE,
                      hojas::SHIRT_OFFSET, VISTAS_CUATRO),
            por_cuadro(brazos_de(player_id, elegida.shirt), brazos_en_hoja),
            por_vista(pelo, hojas::hair_cell(elegida.hairStyle), hojas::FRAME_SIZE,
                      hojas::HAIR_OFFSET, VISTAS_PELO, color_de_pelo(elegida.hairColor)),
    };
    if (elegida.hat != hojas::NO_HAT) {
        resultado.push_back(por_vista(sombreros, hojas::hat_cell(elegida.hat), hojas::HAT_SIZE,
                                      hojas::HAT_OFFSET, VISTAS_CUATRO));
    }
    return resultado;
}

void ConstructorCapas::olvidar(uint32_t player_id) { brazos.erase(player_id); }

SDL2pp::Texture& ConstructorCapas::brazos_de(uint32_t player_id, uint16_t camisa) {
    auto ya_armados = brazos.find(player_id);
    if (ya_armados != brazos.end()) {
        return ya_armados->second;
    }
    std::map<uint32_t, uint32_t> reemplazos = colores_de_manga(pixeles_camisas, camisa);
    SDL2pp::Surface hoja = pixeles_cuerpo.Convert(SDL_PIXELFORMAT_ARGB8888);
    for (int y = 0; y < hoja.GetHeight(); ++y) {
        for (int x = hojas::ARMS_COLUMNS * hojas::FRAME_SIZE.x; x < hoja.GetWidth(); ++x) {
            uint32_t& actual = pixel(hoja, x, y);
            auto reemplazo = reemplazos.find(actual & MASCARA_COLOR);
            if (reemplazo != reemplazos.end()) {
                actual = (actual & MASCARA_ALFA) | reemplazo->second;
            }
        }
    }
    return brazos.try_emplace(player_id, renderer, hoja).first->second;
}
