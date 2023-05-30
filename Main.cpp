#include "StopwatchViewer.h"

#include <iostream>
#include <string>

#include <QApplication>
#include <QtGui>

QCoreApplication* createApplication(int& argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (!qstrcmp(argv[i], "-c")) {
      return new QCoreApplication(argc, argv);
    }
  }
  return new QApplication(argc, argv);
}

int main(int argc, char* argv[]) {
  std::unique_ptr<StopwatchViewer> stopwatchViewer;

  QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

  if (qobject_cast<QApplication*>(app.data())) {
    stopwatchViewer = std::make_unique<StopwatchViewer>(false);
  } else {
    stopwatchViewer = std::make_unique<StopwatchViewer>(true);
  }

  return app->exec();
}
