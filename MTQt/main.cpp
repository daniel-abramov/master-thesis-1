#include "mainwidget.h"

#include <QApplication>
#include <exception>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString imageFilename = argc > 1 ? QString(argv[1]) : QString();
    MainWidget w(imageFilename);
    w.show();

    return a.exec();
}
