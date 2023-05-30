#include <QHeaderView>
#include <QSplitter>
#include <QTimer>

#include "StopwatchDecoder.h"
#include "StopwatchViewer.h"
#include "TerminalViewer.h"

StopwatchViewer::StopwatchViewer(const bool cli) {
  this->resize(1024, 768);
  this->setWindowTitle("StopwatchViewer");
  this->setObjectName("StopwatchViewerUi");

  auto topWidget = new QWidget(this);
  auto topLayout = new QVBoxLayout(this);
  tableWidget_ = new QTableWidget(this);
  topLayout->addWidget(tableWidget_);
  topWidget->setLayout(topLayout);

  auto bottomWidget = new QWidget(this);
  auto bottomLayout = new QVBoxLayout();
  auto vbox = new QVBoxLayout();
  auto hbox = new QHBoxLayout();
  auto flushButton = new QPushButton("Flush Cache");
  plotChoice_ = new QComboBox(this);
  plotHolderWidget_ = new PlotHolderWidget(this);

  hbox->addWidget(flushButton);
  hbox->addWidget(plotChoice_);
  vbox->addWidget(plotHolderWidget_);

  bottomLayout->addLayout(hbox);
  bottomLayout->addLayout(vbox);
  bottomWidget->setLayout(bottomLayout);

  auto splitter = new QSplitter(Qt::Orientation::Vertical, this);
  splitter->addWidget(topWidget);
  splitter->addWidget(bottomWidget);

  auto* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(splitter);
  this->setLayout(mainLayout);

  udpSocket_ = new QUdpSocket(this);
  const bool successfullyBoundPort = udpSocket_->bind(45454, QUdpSocket::DontShareAddress);

  if (!successfullyBoundPort) {
    std::cerr << "Unable to bind port " << 45454 << ", is there a Stopwatch viewer already running?"
              << std::endl;
    exit(-1);
  }

  connect(udpSocket_, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));

  QStringList columnTitles;
  columnTitles << "Item"
               << "Plot"
               << "Last (ms)"
               << "Min (ms)"
               << "Max (ms)"
               << "Avg (ms)"
               << "Hz";

  tableWidget_->setColumnCount(columnTitles.length());
  tableWidget_->setHorizontalHeaderLabels(columnTitles);
  tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  tableWidget_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableWidget_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  tableWidget_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  tableWidget_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  tableWidget_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
  tableWidget_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
  tableWidget_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
  tableWidget_->horizontalHeader()->setTextElideMode(Qt::TextElideMode::ElideMiddle);

  columnTitles.removeFirst();
  columnTitles.removeFirst();

  plotChoice_->addItems(columnTitles);

  connect(flushButton, SIGNAL(clicked()), this, SLOT(flushCache()));

  QMetaObject::connectSlotsByName(this);

  if (cli) {
    terminalViewer_ = std::make_unique<TerminalViewer>();
    QTimer* timer = new QTimer;
    timer->start(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTerminal()));
  }
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

void StopwatchViewer::checkboxHit() {
  int numChecked = 0;

  for (int i = 0; i < tableWidget_->rowCount(); i++) {
    QCheckBox* cellCheckBox = qobject_cast<QCheckBox*>(
        tableWidget_->cellWidget(i, 1)->findChild<QHBoxLayout*>()->itemAt(0)->widget());

    if (cellCheckBox->isChecked()) {
      numChecked++;
    }
  }

  for (int j = 0; j < tableWidget_->rowCount(); j++) {
    QCheckBox* cellCheckBox = qobject_cast<QCheckBox*>(
        tableWidget_->cellWidget(j, 1)->findChild<QHBoxLayout*>()->itemAt(0)->widget());

    cellCheckBox->setCheckable(true);
  }
}

void StopwatchViewer::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    QApplication::quit();
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

    updateTable();
  }
}

void StopwatchViewer::updateTable() {
  int currentNumTimers = 0;

  for (auto it = signatureToNameToDurations_.begin(); it != signatureToNameToDurations_.end();
       it++) {
    currentNumTimers += it->second.size();
  }

  tableWidget_->setRowCount(currentNumTimers);

  std::vector<std::pair<std::string, float>> plotVals;

  for (auto it = signatureToNameToDurations_.begin(); it != signatureToNameToDurations_.end();
       it++) {
    const auto& stopwatch = it->second;

    for (auto it = stopwatch.begin(); it != stopwatch.end(); it++) {
      TableRow& newEntry = const_cast<TableRow&>(it->second.second);

      if (newEntry.isUninit()) {
        newEntry.row = lastRow_++;

        while ((int)newEntry.tableItems.size() < tableWidget_->columnCount() - 1) {
          newEntry.tableItems.emplace_back(new QTableWidgetItem());
        }

        newEntry.tableItems[0]->setText(QString::fromStdString(it->first));
        newEntry.tableItems[1]->setText(QString::number(it->second.first[0]));
        newEntry.tableItems[2]->setText(QString::number(it->second.first.getMinimum()));
        newEntry.tableItems[3]->setText(QString::number(it->second.first.getMaximum()));
        newEntry.tableItems[4]->setText(QString::number(it->second.first.getAverage()));
        newEntry.tableItems[5]->setText(QString::number(it->second.first.getReciprocal() * 1000.0));

        newEntry.checkBoxWidget = new QWidget();
        newEntry.checkItem = new QCheckBox();
        newEntry.layoutCheckBox = new QHBoxLayout(newEntry.checkBoxWidget);
        newEntry.layoutCheckBox->addWidget(newEntry.checkItem);
        newEntry.layoutCheckBox->setAlignment(Qt::AlignCenter);
        newEntry.layoutCheckBox->setContentsMargins(0, 0, 0, 0);

        if (enabledBeforeReset_.count(it->first)) {
          newEntry.checkItem->setChecked(true);
        }

        connect(newEntry.checkItem, SIGNAL(stateChanged(int)), this, SLOT(checkboxHit()));

        tableWidget_->setItem(newEntry.row, 0, newEntry.tableItems[0]);
        tableWidget_->setCellWidget(newEntry.row, 1, newEntry.checkBoxWidget);
        tableWidget_->setItem(newEntry.row, 2, newEntry.tableItems[1]);
        tableWidget_->setItem(newEntry.row, 3, newEntry.tableItems[2]);
        tableWidget_->setItem(newEntry.row, 4, newEntry.tableItems[3]);
        tableWidget_->setItem(newEntry.row, 5, newEntry.tableItems[4]);
        tableWidget_->setItem(newEntry.row, 6, newEntry.tableItems[5]);
      } else {
        tableWidget_->item(newEntry.row, 0)->setText(QString::fromStdString(it->first));
        tableWidget_->item(newEntry.row, 2)->setText(QString::number(it->second.first[0]));
        tableWidget_->item(newEntry.row, 3)
            ->setText(QString::number(it->second.first.getMinimum()));
        tableWidget_->item(newEntry.row, 4)
            ->setText(QString::number(it->second.first.getMaximum()));
        tableWidget_->item(newEntry.row, 5)
            ->setText(QString::number(it->second.first.getAverage()));
        tableWidget_->item(newEntry.row, 6)
            ->setText(QString::number(it->second.first.getReciprocal() * 1000.0));
      }

      for (int i = 0; i < 7; i++) {
        if (i != 1) {
          tableWidget_->item(newEntry.row, i)->setTextAlignment(Qt::AlignCenter);
        }
      }

      QCheckBox* cellCheckBox = qobject_cast<QCheckBox*>(tableWidget_->cellWidget(newEntry.row, 1)
                                                             ->findChild<QHBoxLayout*>()
                                                             ->itemAt(0)
                                                             ->widget());

      if (cellCheckBox->isChecked()) {
        plotVals.emplace_back(
            it->first,
            tableWidget_->item(newEntry.row, plotChoice_->currentIndex() + 2)->text().toDouble());
      }
    }
  }

  plotHolderWidget_->update(plotVals);
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
  updateTable();
  plotHolderWidget_->clear();
}
