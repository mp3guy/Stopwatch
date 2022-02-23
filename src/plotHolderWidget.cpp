/*
 * @author Fiachra & Tom
 */

#include "plotHolderWidget.h"

PlotHolderWidget::PlotHolderWidget(QWidget* parent) : QWidget(parent) {
  setWindowTitle("Plot Window");

  mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  resetButton = new QPushButton("Reset Plot", this);
  mainLayout->addWidget(resetButton);

  plotLength = new QSlider;
  plotLength->setMinimum(50);
  plotLength->setMaximum(10000);
  plotLength->setOrientation(Qt::Horizontal);
  mainLayout->addWidget(plotLength);

  currentDataLength = new QLabel;
  currentDataLength->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  currentDataLength->setText(
      "Current Window Sample Length: " + QString::number(DataPlotWidget::DEFAULT_DATA_LENGTH));
  mainLayout->addWidget(currentDataLength);

  plotLayout = new QVBoxLayout;
  dataPlotWidget = new DataPlotWidget();
  plotLayout->addWidget(dataPlotWidget);
  mainLayout->addLayout(plotLayout);

  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetPlot()));
  connect(plotLength, SIGNAL(valueChanged(int)), this, SLOT(changeDataLength(int)));
}

void PlotHolderWidget::update(float* values, bool* enabled) {
  dataPlotWidget->updatePlot(values, enabled);
}

void PlotHolderWidget::resetPlot() {
  dataPlotWidget->resetPlot();
}

void PlotHolderWidget::changeDataLength(int len) {
  dataPlotWidget->setDataLength(len);
  currentDataLength->setText("Current Window Sample Length: " + QString::number(len));
}
