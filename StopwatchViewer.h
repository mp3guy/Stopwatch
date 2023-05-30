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

 private:
  QUdpSocket* udpSocket_ = nullptr;
  bool plotAverages_ = false;
  std::unique_ptr<TerminalViewer> terminalViewer_;

  PlotHolderWidget* plotHolderWidget_ = nullptr;
  QTableWidget* tableWidget_ = nullptr;
  QComboBox* plotChoice_ = nullptr;

  std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>
      signatureToNameToDurations_;
  std::map<uint64_t, std::map<std::string, uint64_t>> signatureToNameToTicksUs_;

  int lastRow_ = 0;

  void updateTable();
  void keyPressEvent(QKeyEvent* event) override;

  std::unordered_set<std::string> enabledBeforeReset_;

 private slots:
  void processPendingDatagram();
  void flushCache();
  void checkboxHit();
  void updateTerminal();
};
