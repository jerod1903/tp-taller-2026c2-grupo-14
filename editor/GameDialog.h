#pragma once

#include <QDialog>
#include <QString>

class MenuButton;
class QGridLayout;

// Un cartel de madera como los del menu del juego, sin barra de titulo: un titulo, una aclaracion,
// los campos que pone cada dialogo y Aceptar y Cancelar.
class GameDialog: public QDialog {
protected:
    GameDialog(const QString& title, const QString& note, QWidget* parent);
    // Agrega un campo debajo de los anteriores, con su etiqueta a la izquierda y centrada en su
    // misma linea
    void addField(const QString& label, QWidget* field);
    void setAcceptEnabled(bool enabled);

private:
    QGridLayout* fields;
    MenuButton* acceptButton;
};
