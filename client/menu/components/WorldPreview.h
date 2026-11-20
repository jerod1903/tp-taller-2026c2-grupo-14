#pragma once

#include <QImage>
#include <optional>
#include <string>

// La vista previa de un mundo en el menú: su mapa general (cargar_mundo + grilla_del_mapa), una
// casilla por píxel, con los mismos colores que el mapa del juego. Vacío si el mapa no se puede
// leer (por ejemplo, si no está en esta máquina).
std::optional<QImage> worldPreviewImage(const std::string& worldId);
