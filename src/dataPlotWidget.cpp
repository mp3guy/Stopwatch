#include <cmath>
#include <iostream>

#include <QApplication>
#include <QPainter>
#include <QResizeEvent>

#include "dataPlotWidget.h"

DataPlotWidget::DataPlotWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(plotWidth, plotHeight);
}

void DataPlotWidget::updatePlot(const std::vector<std::pair<std::string, float>>& plotVals) {
  for (auto& nameVal : plotVals) {
    if (dataArray.count(nameVal.first) == 0) {
      auto data = dataArray.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(nameVal.first),
          std::forward_as_tuple(std::make_pair<QColor, std::vector<float>>(
              QColor::fromHsvF(std::fmod(colorCounter++ * 0.5 * (1 + std::sqrt(5)), 1), 1, 1),
              {})));
      data.first->second.second.resize(maxPlotLength, std::numeric_limits<float>::quiet_NaN());
    }
  }

  if (currentCount == currentIndex) {
    if (currentCount < maxPlotLength) {
      for (auto& nameVal : plotVals) {
        auto& data = dataArray[nameVal.first].second;
        data[currentIndex] = nameVal.second;
      }

      currentCount++;
      currentIndex++;
    } else {
      for (auto& nameVal : plotVals) {
        auto& data = dataArray[nameVal.first].second;
        data.erase(data.begin());
        data.push_back(nameVal.second);
      }
    }
  } else {
    currentIndex++;
  }

  // Remove no-longer plot values
  for (auto it = dataArray.cbegin(); it != dataArray.cend();) {
    if (std::find_if(plotVals.begin(), plotVals.end(), [&](const auto& element) {
          return it->first == element.first;
        }) == plotVals.end()) {
      dataArray.erase(it++);
    } else {
      ++it;
    }
  }

  update();
}

void DataPlotWidget::paintEvent(QPaintEvent* event) {
  if (event == 0)
    return;

  painter.begin(this);
  drawDataPlot();
  painter.end();
}

int DataPlotWidget::getPlotY(float val, float dataMin, float dataMax) {
  float dataRange;

  if (dataMax == dataMin) {
    dataMax += dataMax * 0.1;
    dataMin -= dataMin * 0.1;
  }

  dataRange = dataMax - dataMin;
  dataRange *= 1.1;

  int yVal = ((val - dataMin) * ((plotHeight - 13) / dataRange)) + (13);

  return yVal;
}

void DataPlotWidget::drawDataPlot() {
  painter.setBrush(QColor("black"));
  painter.drawRect(0, 0, plotWidth, plotHeight);

  painter.setPen(QPen(QColor("green"), 1));
  float dataMin = std::numeric_limits<float>::max();
  float dataMax = 0.0f;

  for (const auto& data : dataArray) {
    for (int i = 0; i < currentCount; i++) {
      if (!std::isfinite(data.second.second[i])) {
        continue;
      }

      dataMax = std::max(dataMax, data.second.second[i]);
      dataMin = std::min(dataMin, data.second.second[i]);
    }
  }

  int lastDrawnHeight = 18;
  const int textHeight = 24;
  painter.drawText(5, lastDrawnHeight, "Max: " + QString::number(dataMax));
  painter.drawText(5, plotHeight - 5, "Min: " + QString::number(dataMin));
  lastDrawnHeight += textHeight;

  QTransform inverted(1, 0, 0, 0, -1, 0, 0, 0, 1);
  inverted.translate(0, -plotHeight);

  for (const auto& data : dataArray) {
    painter.setPen(QPen(data.second.first, 1));

    painter.resetTransform();
    painter.drawText(5, lastDrawnHeight, QString::fromStdString(data.first));
    lastDrawnHeight += textHeight;

    painter.setTransform(inverted);
    for (int i = 1; i < currentCount; i++) {
      if (std::isfinite(data.second.second[i - 1]) && std::isfinite(data.second.second[i])) {
        painter.drawLine(
            (int)(((float)plotWidth / (float)maxPlotLength) * (i - 1)),
            getPlotY(data.second.second[i - 1], dataMin, dataMax),
            (int)(((float)plotWidth / (float)maxPlotLength) * (i)),
            getPlotY(data.second.second[i], dataMin, dataMax));
      }
    }
  }
}

void DataPlotWidget::resetPlot() {
  currentCount = 0;
  currentIndex = 0;

  dataArray.clear();
}

void DataPlotWidget::setDataLength(int newLength) {
  for (auto& data : dataArray) {
    if (data.second.second.size() > newLength) {
      const int shrinkAmount = data.second.second.size() - newLength;
      // Shrink buffer by removing oldest data at the start
      data.second.second.erase(
          data.second.second.begin(), data.second.second.begin() + shrinkAmount);
      currentIndex -= shrinkAmount;
      currentCount -= shrinkAmount;

      currentIndex = std::max(0, currentIndex);
      currentCount = std::max(0, currentCount);
    } else {
      // Grow buffer by adding space for more measurements at the end
      data.second.second.resize(newLength, std::numeric_limits<float>::quiet_NaN());
    }
  }

  maxPlotLength = newLength;
}

void DataPlotWidget::resizeEvent(QResizeEvent* event) {
  plotWidth = event->size().width();
  plotHeight = event->size().height();
}
