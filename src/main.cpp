#include <QApplication>
#include <QIcon>
#include "AppWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/appicon.png"));
    AppWindow window;
    window.show();
    return app.exec();
} 