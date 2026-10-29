#pragma once

#include <QWidget>

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

protected:
    QLabel* createTitle(const QString& text);
    // Agrega el boton "Volver" como primera fila; al tocarlo emite backRequested().
    void addBackButton();
    // Titulo en la misma fila que "Volver" (si la hay), centrado.
    void setTitle(const QString& text);

    QVBoxLayout* mainLayout;
    QHBoxLayout* headerRow = nullptr;
};
