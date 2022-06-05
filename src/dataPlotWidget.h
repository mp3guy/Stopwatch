/*
 * @author fiachra & tom
 */

#pragma once

#include <deque>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWidget>

class QPaintEvent;

class DataPlotWidget : public QWidget {
  Q_OBJECT;

 public:
  DataPlotWidget(QWidget* parent = 0);
  virtual ~DataPlotWidget() = default;

  void updatePlot(const std::vector<std::pair<std::string, float>>& plotVals);
  void setDataLength(int newLength);
  void resetPlot();

  int plotWidth = 640;
  int plotHeight = 240;

  static constexpr int kInitialPlotLength = 100;

 private:
  QPainter painter;

  void paintEvent(QPaintEvent* event) override;
  int getPlotY(float val, float dataMin, float dataMax);
  void drawDataPlot();
  void resizeEvent(QResizeEvent* event) override;

  int maxPlotLength = kInitialPlotLength;
  int currentIndex = 0;
  int currentCount = 0;
  int colorCounter = 0;
  std::map<std::string, std::pair<QColor, std::vector<float>>> dataArray;
};
