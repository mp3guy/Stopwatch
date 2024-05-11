#pragma once

#include <unordered_set>

#include <QApplication>
#include <QCheckBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTableWidget>
#include <QUdpSocket>

#include "PlotHolderWidget.h"
#include "RingBuffer.h"

class TableRow {
 public:
  TableRow() {}

  inline bool isUninit() const {
    return tableItems.empty();
  }

  int row = 0;
  std::vector<QTableWidgetItem*> tableItems;
};

class ViewerWidget : public QWidget {
  Q_OBJECT

 public:
  ViewerWidget();

  QPushButton* flushButton = nullptr;
  PlotHolderWidget* plotHolderWidget_ = nullptr;
  QTableWidget* tableWidget_ = nullptr;
  QSortFilterProxyModel* proxy_ = nullptr;
  QTableView* tableView_ = nullptr;
  QComboBox* plotChoice_ = nullptr;
  QLineEdit* filterText_;

  void keyPressEvent(QKeyEvent* event) override;

  void updateTable(
      const std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>&
          signatureToNameToDurations,
      const std::unordered_set<std::string>& enabledBeforeReset,
      int& lastRow);
};
