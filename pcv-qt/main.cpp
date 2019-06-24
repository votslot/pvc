#include "mainwindow.h"
#include <QApplication>

extern int main123(int argc, char **argv);
extern void RunQuad();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    RunQuad();
    int ret = a.exec();

    return ret;
}
