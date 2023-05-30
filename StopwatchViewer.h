#pragma once

#include <iostream>
#include <map>
#include <unordered_set>
#include <utility>

#include "ViewerWidget.h"

class TerminalViewer;

class StopwatchViewer : public QObject {
  Q_OBJECT

 public:
  StopwatchViewer(const bool cli);
  ~StopwatchViewer();

 private:
  ViewerWidget* viewerWidget_ = nullptr;

  QUdpSocket* udpSocket_ = nullptr;
  bool plotAverages_ = false;
  std::unique_ptr<TerminalViewer> terminalViewer_;

  std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>
      signatureToNameToDurations_;
  std::map<uint64_t, std::map<std::string, uint64_t>> signatureToNameToTicksUs_;

  int lastRow_ = 0;

  std::unordered_set<std::string> enabledBeforeReset_;

 private slots:
  void processPendingDatagram();
  void flushCache();
  void updateTerminal();
};
