#pragma once

#include <QMessageBox>
#include <QString>

class QWidget;

// Los cuadros de mensaje del editor, con los botones en castellano escritos aca: los de Qt salen en
// ingles si no estan instaladas sus traducciones.

// Pregunta y devuelve si se respondio Si. La respuesta por defecto es No.
bool ask(QWidget* parent, const QString& title, const QString& text);

// Un aviso con un solo boton, Aceptar. El icono dice si es informacion, advertencia o error. Ancho,
// para una lista de renglones largos (los errores del validador) que no se corten.
void tell(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
          bool wide = false);
