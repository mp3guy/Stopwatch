/*
 * @author Fiachra & Tom
 */

#include "plotHolderWidget.h"

PlotHolderWidget::PlotHolderWidget(QWidget *parent)
 : QWidget(parent)
{
    mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    setWindowTitle("Plot Window");

    dataPlotWidget = new DataPlotWidget();

    plotLayout = new QVBoxLayout;
    buttonLayout = new QHBoxLayout;

    currentDataLength = new QLabel;
    currentDataLength->setText("Current Window Sample Length: " + QString::number(DataPlotWidget::DEFAULT_DATA_LENGTH));

    plotLength = new QSlider;
    plotLength->setMinimum(50);
    plotLength->setMaximum(10000);
    plotLength->setOrientation(Qt::Horizontal);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(plotLength);
    mainLayout->addWidget(currentDataLength);

    resetButton = new QPushButton("Reset Plot", this);
    buttonLayout->addWidget(resetButton);

    plotLayout->addWidget(dataPlotWidget);
    mainLayout->addLayout(plotLayout);

    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetPlot()));
    connect(plotLength, SIGNAL(valueChanged(int)), this, SLOT(changeDataLength(int)));
}

void PlotHolderWidget::update(float * values, bool * enabled)
{
    dataPlotWidget->updatePlot(values, enabled);
}

void PlotHolderWidget::resetPlot()
{
    dataPlotWidget->resetPlot();
}

void PlotHolderWidget::changeDataLength(int len)
{
    dataPlotWidget->setDataLength(len);
    currentDataLength->setText("Current Window Sample Length: " + QString::number(len));
}
