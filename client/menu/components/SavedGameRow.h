#pragma once

#include <QWidget>

#include "client/menu/backend/LobbyTypes.h"

// Fila de la lista "Mis partidas": imagen del personaje a la izquierda y los
// datos a la derecha (partida, personaje, multijugador).
class SavedGameRow: public QWidget {
public:
    explicit SavedGameRow(const SavedGameInfo& game, QWidget* parent = nullptr);
};
