#pragma once

#include <iostream>
#include <map>
#include <unordered_set>
#include <utility>

#include <QApplication>
#include <QCheckBox>
#include <QKeyEvent>
#include <QTableWidget>
#include <QUdpSocket>

#include "PlotHolderWidget.h"
#include "RingBuffer.h"

class TerminalViewer;

class StopwatchViewer : public QWidget {
  Q_OBJECT

 public:
  StopwatchViewer(const bool cli);
  ~StopwatchViewer();

 private:
  QUdpSocket* udpSocket_ = nullptr;
  bool plotAverages_ = false;
  std::unique_ptr<TerminalViewer> terminalViewer_;

  class TableRow {
   public:
    TableRow() {}

    virtual ~TableRow() {
      delete[] tableItems;
      delete layoutCheckBox;
      delete checkBoxWidget;
    }

    inline bool isUninit() const {
      return tableItems == 0 || checkItem == 0;
    }

    int row = 0;
    QTableWidgetItem* tableItems = nullptr;
    QCheckBox* checkItem = nullptr;
    QWidget* checkBoxWidget = nullptr;
    QHBoxLayout* layoutCheckBox = nullptr;
  };

  PlotHolderWidget* plotHolderWidget_ = nullptr;
  QTableWidget* tableWidget_ = nullptr;
  QComboBox* plotChoice_ = nullptr;

  std::map<
      uint64_t,
      std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>>
      cache_;

  int lastRow_ = 0;

  void updateTable();
  void keyPressEvent(QKeyEvent* event) override;

  std::unordered_set<std::string> enabledBeforeReset_;

 private slots:
  void processPendingDatagram();
  void flushCache();
  void checkboxHit();
};
