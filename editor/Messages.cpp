#include "editor/Messages.h"

#include <QPushButton>

namespace {
// Lo justo para que un error del validador con todos sus datos entre en un renglon
constexpr int WIDE_TEXT = 640;
}  // namespace

bool ask(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    QPushButton* yes = box.addButton("Sí", QMessageBox::YesRole);
    QPushButton* no = box.addButton("No", QMessageBox::NoRole);
    box.setDefaultButton(no);
    box.exec();
    return box.clickedButton() == yes;
}

void tell(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
          bool wide) {
    QMessageBox box(icon, title, text, QMessageBox::NoButton, parent);
    box.addButton("Aceptar", QMessageBox::AcceptRole);
    if (wide) {
        // qt_msgbox_label es el texto del cuadro; el icono es otro QLabel y queda como esta
        box.setStyleSheet(QString("QLabel#qt_msgbox_label { min-width: %1px; }").arg(WIDE_TEXT));
    }
    box.exec();
}
