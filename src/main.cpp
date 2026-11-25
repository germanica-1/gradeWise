#include <QApplication>
#include <QFile>
#include <QStyle>
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("GwaCalculator"); // also names the data folder
    QApplication::setStyle("Fusion");                  // same look on Linux and Windows
    app.setPalette(app.style()->standardPalette());    // light colors even when the OS uses a dark theme

    QFile style(":/style.qss");
    if (style.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(style.readAll()));
    }

    MainWindow window;
    window.show();
    return app.exec();
}
