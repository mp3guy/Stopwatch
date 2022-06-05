/*
 * @author fiachra & tom
 */

#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>
#include <map>
#include <string>

#include "dataPlotWidget.h"

class QPaintEvent;

class PlotHolderWidget : public QWidget {
  Q_OBJECT;

 public:
  PlotHolderWidget(QWidget* parent = 0);
  void update(const std::vector<std::pair<std::string, float>>& plotVals);
  void clear();

  DataPlotWidget* dataPlotWidget;
  QSlider* plotLength;
  QLabel* currentDataLength;
  QVBoxLayout* mainLayout;
  QVBoxLayout* plotLayout;
  QHBoxLayout* lengthLayout;

 public slots:
  void changeDataLength(int len);
};
