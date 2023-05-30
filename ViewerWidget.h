#pragma once

#include <unordered_set>

#include <QApplication>
#include <QCheckBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QUdpSocket>

#include "PlotHolderWidget.h"
#include "RingBuffer.h"

class TableRow {
 public:
  TableRow() {}

  inline bool isUninit() const {
    return tableItems.empty() || checkItem == nullptr;
  }

  int row = 0;
  std::vector<QTableWidgetItem*> tableItems;
  QCheckBox* checkItem = nullptr;
  QWidget* checkBoxWidget = nullptr;
  QHBoxLayout* layoutCheckBox = nullptr;
};

class ViewerWidget : public QWidget {
  Q_OBJECT

 public:
  ViewerWidget();

  QPushButton* flushButton = nullptr;
  PlotHolderWidget* plotHolderWidget_ = nullptr;
  QTableWidget* tableWidget_ = nullptr;
  QComboBox* plotChoice_ = nullptr;

  void keyPressEvent(QKeyEvent* event) override;

  void updateTable(
      const std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>&
          signatureToNameToDurations,
      const std::unordered_set<std::string>& enabledBeforeReset,
      int& lastRow);

 private slots:
  void checkboxHit();
};
