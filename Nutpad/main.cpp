#include "Nutpad.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Nutpad w;
    w.show();
    return a.exec();
}
