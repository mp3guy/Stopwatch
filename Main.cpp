#include "StopwatchViewer.h"

#include <iostream>
#include <string>

#include <QApplication>
#include <QtGui>

int main(int argc, char* argv[]) {
  bool useTerminal = false;

  if (argc >= 2) {
    for (int i = 0; i < argc; ++i) {
      const std::string arg(argv[i]);
      if (arg == "-c") {
        useTerminal = true;
        break;
      }
    }
  }

  QApplication a(argc, argv);
  StopwatchViewer w(useTerminal);
  w.show();
  return a.exec();
}
