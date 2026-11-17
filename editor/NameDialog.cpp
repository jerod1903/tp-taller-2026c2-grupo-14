#include "editor/NameDialog.h"

#include <QLineEdit>

#include "editor/Guidelines.h"

namespace {
constexpr int INPUT_WIDTH = 420;
}  // namespace

NameDialog::NameDialog(const stardew::maps::MapRequirements& requirements, QWidget* parent):
        GameDialog("Nuevo mapa",
                   "Comienza con una granja y un pueblo unidos por un pasaje.\n\n" +
                           QString::fromStdString(mapGuidelines(requirements)),
                   parent),
        nameInput(new QLineEdit(this)) {
    nameInput->setFixedWidth(INPUT_WIDTH);
    addField("Nombre", nameInput);
    // Sin nombre no se puede aceptar
    setAcceptEnabled(false);
    connect(nameInput, &QLineEdit::textChanged, this,
            [this]() { setAcceptEnabled(!name().isEmpty()); });
}

QString NameDialog::name() const { return nameInput->text().trimmed(); }
