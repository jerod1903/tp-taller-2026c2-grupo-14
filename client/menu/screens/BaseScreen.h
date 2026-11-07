#pragma once

#include <QWidget>
#include <optional>

#include "client/menu/backend/ILobbyProtocol.h"

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

// Base de todas las pantallas: layout vertical con margenes comunes y helpers
// para el titulo y el boton "Volver" (arriba a la izquierda).
class BaseScreen: public QWidget {
    Q_OBJECT

public:
    explicit BaseScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
    // Se corto la conexion con el servidor: MainWindow avisa y cierra el cliente.
    void connectionLost();

protected:
    // Hace una llamada al servidor. Si se corta la conexion emite connectionLost y devuelve
    // nullopt: la excepcion no puede salir del slot, porque atravesaria el codigo de Qt.
    template <typename Call>
    auto guarded(Call&& call) -> std::optional<decltype(call())> {
        try {
            return call();
        } catch (const LobbyConnectionError&) {
            emit connectionLost();
            return std::nullopt;
        }
    }

    QLabel* createTitle(const QString& text);
    // Agrega el boton "Volver" como primera fila; al tocarlo emite backRequested().
    void addBackButton();
    // Titulo en la misma fila que "Volver" (si la hay), centrado.
    void setTitle(const QString& text);

    QVBoxLayout* mainLayout;
    QHBoxLayout* headerRow = nullptr;
};
