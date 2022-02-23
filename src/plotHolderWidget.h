/*
 * @author fiachra & tom
 */

#ifndef PLOT_HOLDER_WIDGET_H
#define PLOT_HOLDER_WIDGET_H

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

#include "RingBuffer.h"
#include "dataPlotWidget.h"

class QPaintEvent;

class PlotHolderWidget : public QWidget {
  Q_OBJECT;

 public:
  PlotHolderWidget(QWidget* parent = 0);
  void update(float* values, bool* enabled);

  DataPlotWidget* dataPlotWidget;
  QSlider* plotLength;
  QLabel* currentDataLength;
  QPushButton* resetButton;
  QVBoxLayout* mainLayout;
  QVBoxLayout* plotLayout;
  QHBoxLayout* lengthLayout;

 public slots:
  void resetPlot();
  void changeDataLength(int len);
};

#endif
