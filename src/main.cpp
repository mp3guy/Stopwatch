#include "stopwatchviewer.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StopwatchViewer w;
    w.show();
    return a.exec();
}
