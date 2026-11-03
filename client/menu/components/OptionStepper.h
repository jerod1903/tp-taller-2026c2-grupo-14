#pragma once

#include <QString>
#include <QWidget>

class QLabel;

// Selector "‹ Camisa 3 / 30 ›" para elegir una opcion de una lista numerada.
// Las flechas dan la vuelta: despues de la ultima viene la primera.
class OptionStepper: public QWidget {
    Q_OBJECT

public:
    // count: cantidad de opciones (valores de 0 a count - 1).
    // noneText: si no esta vacio, es lo que se muestra para el valor 0 (por ej. "Sin sombrero").
    OptionStepper(const QString& title, int count, const QString& noneText = QString(),
                  QWidget* parent = nullptr);

    int value() const;
    void setValue(int newValue);

signals:
    void valueChanged(int value);

private:
    void step(int delta);
    void refreshText();

    const QString title;
    const int count;
    const QString noneText;
    int current = 0;
    QLabel* valueLabel;
};
