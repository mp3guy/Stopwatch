/*
 * @author Fiachra & Tom
 */

#include "plotHolderWidget.h"

PlotHolderWidget::PlotHolderWidget(QWidget* parent) : QWidget(parent) {
  setWindowTitle("Plot Window");

  mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  plotLength = new QSlider;
  plotLength->setMinimum(50);
  plotLength->setMaximum(10000);
  plotLength->setOrientation(Qt::Horizontal);
  mainLayout->addWidget(plotLength);

  currentDataLength = new QLabel;
  currentDataLength->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  currentDataLength->setText(
      "Current Window Sample Length: " + QString::number(DataPlotWidget::kInitialPlotLength));
  mainLayout->addWidget(currentDataLength);

  plotLayout = new QVBoxLayout;
  dataPlotWidget = new DataPlotWidget();
  plotLayout->addWidget(dataPlotWidget);
  mainLayout->addLayout(plotLayout);

  connect(plotLength, SIGNAL(valueChanged(int)), this, SLOT(changeDataLength(int)));
}

void PlotHolderWidget::update(const std::vector<std::pair<std::string, float>>& plotVals) {
  dataPlotWidget->updatePlot(plotVals);
}

void PlotHolderWidget::changeDataLength(int len) {
  dataPlotWidget->setDataLength(len);
  currentDataLength->setText("Current Window Sample Length: " + QString::number(len));
}

void PlotHolderWidget::clear() {
  dataPlotWidget->resetPlot();
}
