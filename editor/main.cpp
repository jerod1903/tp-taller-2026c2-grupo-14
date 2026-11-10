#include <QApplication>
#include <exception>
#include <iostream>

#include "editor/EditorWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        // Lee los requisitos de los mapas: sin ellos no se puede validar
        EditorWindow window;
        window.show();
        return QApplication::exec();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
