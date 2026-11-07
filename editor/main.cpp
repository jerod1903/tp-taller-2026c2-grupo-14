#include <QApplication>

#include "editor/EditorWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    EditorWindow window;
    window.show();
    return QApplication::exec();
}
