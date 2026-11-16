#pragma once

#include "common/map/MapDefinition.h"

// El interior de la cabaña es el mismo en todos los mundos: la sala sale de
// maps/interiors/cabin.toml y el cuarto de las camas se arma a su derecha segun cuantos jugadores
// entran. Lo usan el servidor (para armar la Cabin) y el cliente (para dibujarla), asi que los dos
// ven lo mismo.
namespace stardew::maps {

inline constexpr const char* CABIN_INTERIOR_FILE = TALLER_MAPS_DIR "/interiors/cabin.toml";

// Lo que ocupa el dibujo de una cama, en casillas, desde su tile sleepable (la esquina de arriba a
// la izquierda): la cabecera arriba y la frazada abajo. Se puede pisar. Su ultima fila termina en
// negro, asi que va sobre el zocalo de abajo del cuarto
inline constexpr int BED_WIDTH = 2;
inline constexpr int BED_HEIGHT = 4;
inline constexpr int BED_HEAD_HEIGHT = 2;

// Las camas van todas en una fila contra la pared de abajo, con una casilla libre entre cama y
// cama. Delante quedan un pasillo y una fila mas para decorar
inline constexpr int BEDROOM_COLUMNS_PER_BED = BED_WIDTH + 1;
inline constexpr int BEDROOM_FREE_ROWS = 2;

// Agrega el cuarto a la derecha del area, solo con su piso y un tile sleepable por jugador: las
// paredes las pone despues addWalls. El cuarto crece hacia la derecha, termina abajo donde termina
// el piso de la sala (sin contar su pasillo de entrada). La ultima columna de la sala es la pared
// que los separa: su casilla de piso es la puerta, y tiene que caer en una fila del cuarto. Lanza
// std::invalid_argument si el cuarto no entra arriba
void addBedroom(MapArea& area, int maxPlayers);

// Lee la sala, le agrega el cuarto y arma las paredes. Lanza std::runtime_error si el archivo no
// se puede leer o no tiene exactamente un area
MapDefinition loadCabinInterior(int maxPlayers);

}  // namespace stardew::maps
