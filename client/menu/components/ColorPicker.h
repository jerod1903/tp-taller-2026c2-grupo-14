#pragma once

#include <QWidget>

class QButtonGroup;

// Grilla de muestras con la paleta del personaje (CharacterColors::PALETTE).
// El valor es el indice en la paleta, el mismo que viaja en CharacterAppearance::hairColor.
class ColorPicker: public QWidget {
    Q_OBJECT

public:
    explicit ColorPicker(QWidget* parent = nullptr);

    int value() const;
    void setValue(int index);

signals:
    void valueChanged(int index);

private:
    QButtonGroup* swatches;
};
