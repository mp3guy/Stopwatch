#include "stopwatchviewer.h"

#include <iostream>
#include <string>

#include <QApplication>
#include <QtGui>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  StopwatchViewer w;
  w.show();
  return a.exec();
}
