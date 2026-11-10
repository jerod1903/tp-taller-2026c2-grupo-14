#include "editor/SizeDialog.h"

#include <QIntValidator>
#include <QLineEdit>

namespace {
constexpr int INPUT_WIDTH = 120;

QLineEdit* sideInput(const SizeRequest& request, int value, QWidget* parent) {
    auto* input = new QLineEdit(QString::number(value), parent);
    input->setValidator(new QIntValidator(request.minSide, request.maxSide, input));
    input->setFixedWidth(INPUT_WIDTH);
    return input;
}
}  // namespace

SizeDialog::SizeDialog(const SizeRequest& request, QWidget* parent):
        GameDialog(request.title, request.note, parent),
        widthInput(sideInput(request, request.initial.width, this)),
        heightInput(sideInput(request, request.initial.height, this)) {
    const QString range = QString(" (%1 a %2)").arg(request.minSide).arg(request.maxSide);
    addField("Ancho" + range, widthInput);
    addField("Alto" + range, heightInput);

    connect(widthInput, &QLineEdit::textChanged, this, &SizeDialog::updateAccept);
    connect(heightInput, &QLineEdit::textChanged, this, &SizeDialog::updateAccept);
}

AreaSize SizeDialog::size() const {
    return {widthInput->text().toInt(), heightInput->text().toInt()};
}

void SizeDialog::updateAccept() {
    setAcceptEnabled(widthInput->hasAcceptableInput() && heightInput->hasAcceptableInput());
}
