#include <QTimer>

#include "StopwatchDecoder.h"
#include "StopwatchViewer.h"
#include "TerminalViewer.h"

StopwatchViewer::StopwatchViewer(const bool cli) {
  udpSocket_ = new QUdpSocket(this);
  const bool successfullyBoundPort = udpSocket_->bind(45454, QUdpSocket::DontShareAddress);

  if (!successfullyBoundPort) {
    std::cerr << "Unable to bind port " << 45454 << ", is there a Stopwatch viewer already running?"
              << std::endl;
    exit(-1);
  }

  connect(udpSocket_, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));

  if (cli) {
    terminalViewer_ = std::make_unique<TerminalViewer>();
    QTimer* timer = new QTimer;
    timer->start(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTerminal()));
  } else {
    viewerWidget_ = new ViewerWidget();
    connect(viewerWidget_->flushButton, SIGNAL(clicked()), this, SLOT(flushCache()));
    QMetaObject::connectSlotsByName(viewerWidget_);
    viewerWidget_->show();
  }

  QMetaObject::connectSlotsByName(this);
}

StopwatchViewer::~StopwatchViewer() {
  udpSocket_->close();
}

void StopwatchViewer::updateTerminal() {
  if (terminalViewer_) {
    const auto [keepRunning, shouldFlush] =
        terminalViewer_->renderUpdate(signatureToNameToDurations_);

    if (!keepRunning) {
      terminalViewer_.reset();
      QApplication::quit();
    } else if (shouldFlush) {
      flushCache();
    }
  }
}

void StopwatchViewer::processPendingDatagram() {
  QByteArray datagram;
  datagram.resize(udpSocket_->pendingDatagramSize());
  udpSocket_->readDatagram(datagram.data(), datagram.size());

  int* data = (int*)datagram.data();

  if (datagram.size() > 0 && datagram.size() == data[0]) {
    std::pair<uint64_t, std::vector<std::pair<std::string, float>>> signatureNameDurations =
        StopwatchDecoder::decodePacket(
            (uint8_t*)datagram.data(), datagram.size(), signatureToNameToTicksUs_);

    auto& nameToDurations = signatureToNameToDurations_[signatureNameDurations.first];

    for (size_t i = 0; i < signatureNameDurations.second.size(); i++) {
      nameToDurations[signatureNameDurations.second.at(i).first].first.add(
          signatureNameDurations.second.at(i).second);
    }

    if (viewerWidget_) {
      viewerWidget_->updateTable(signatureToNameToDurations_, enabledBeforeReset_, lastRow_);
    }
  }
}

void StopwatchViewer::flushCache() {
  enabledBeforeReset_.clear();

  for (auto& cacheIt : signatureToNameToDurations_) {
    for (auto& stopIt : cacheIt.second) {
      if (stopIt.second.second.checkItem && stopIt.second.second.checkItem->isChecked()) {
        enabledBeforeReset_.insert(stopIt.first);
      }
    }
  }

  signatureToNameToDurations_.clear();
  signatureToNameToTicksUs_.clear();
  lastRow_ = 0;

  if (viewerWidget_) {
    viewerWidget_->updateTable(signatureToNameToDurations_, enabledBeforeReset_, lastRow_);
    viewerWidget_->plotHolderWidget_->clear();
  }
}
