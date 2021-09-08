#include "MicroscopeManagerGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MicroscopeManagerGUI w;
    w.show();
    return a.exec();
}
