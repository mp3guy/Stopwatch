#include <QHeaderView>
#include <QSplitter>

#include "StopwatchDecoder.h"
#include "StopwatchViewer.h"

StopwatchViewer::StopwatchViewer(const bool cli) : cli_(cli) {
  this->resize(1024, 768);
  this->setWindowTitle("StopwatchViewer");
  this->setObjectName("StopwatchViewerUi");

  auto topWidget = new QWidget(this);
  auto topLayout = new QVBoxLayout(this);
  tableWidget = new QTableWidget(this);
  topLayout->addWidget(tableWidget);
  topWidget->setLayout(topLayout);

  auto bottomWidget = new QWidget(this);
  auto bottomLayout = new QVBoxLayout();
  auto vbox = new QVBoxLayout();
  auto hbox = new QHBoxLayout();
  auto flushButton = new QPushButton("Flush Cache");
  plotChoice = new QComboBox(this);
  plotHolderWidget = new PlotHolderWidget(this);

  hbox->addWidget(flushButton);
  hbox->addWidget(plotChoice);
  vbox->addWidget(plotHolderWidget);

  bottomLayout->addLayout(hbox);
  bottomLayout->addLayout(vbox);
  bottomWidget->setLayout(bottomLayout);

  auto splitter = new QSplitter(Qt::Orientation::Vertical, this);
  splitter->addWidget(topWidget);
  splitter->addWidget(bottomWidget);

  auto* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(splitter);
  this->setLayout(mainLayout);

  udpSocket = new QUdpSocket(this);
  udpSocket->bind(45454, QUdpSocket::ShareAddress);
  connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));

  QStringList columnTitles;
  columnTitles << "Item"
               << "Plot"
               << "Last (ms)"
               << "Min (ms)"
               << "Max (ms)"
               << "Avg (ms)"
               << "Hz";

  tableWidget->setColumnCount(columnTitles.length());
  tableWidget->setHorizontalHeaderLabels(columnTitles);
  tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

  tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
  tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
  tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);

  columnTitles.removeFirst();
  columnTitles.removeFirst();

  plotChoice->addItems(columnTitles);

  connect(flushButton, SIGNAL(clicked()), this, SLOT(flushCache()));

  QMetaObject::connectSlotsByName(this);
}

StopwatchViewer::~StopwatchViewer() {
  udpSocket->close();
}

void StopwatchViewer::checkboxHit() {
  int numChecked = 0;

  for (int i = 0; i < tableWidget->rowCount(); i++) {
    QCheckBox* cellCheckBox = qobject_cast<QCheckBox*>(tableWidget->cellWidget(i, 1));

    if (cellCheckBox->isChecked()) {
      numChecked++;
    }
  }

  for (int j = 0; j < tableWidget->rowCount(); j++) {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(tableWidget->cellWidget(j, 1));
    checkBox->setCheckable(true);
  }
}

void StopwatchViewer::keyPressEvent(QKeyEvent* event) {
  if (event->key() == 16777216) {
    this->close();
  }
}

void StopwatchViewer::processPendingDatagram() {
  QByteArray datagram;
  datagram.resize(udpSocket->pendingDatagramSize());
  udpSocket->readDatagram(datagram.data(), datagram.size());

  int* data = (int*)datagram.data();

  if (datagram.size() > 0 && datagram.size() == data[0]) {
    std::pair<uint64_t, std::vector<std::pair<std::string, float>>> currentTimes =
        StopwatchDecoder::decodePacket((unsigned char*)datagram.data(), datagram.size());

    std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>&
        stopwatch = cache[currentTimes.first];

    for (unsigned int i = 0; i < currentTimes.second.size(); i++) {
      stopwatch[currentTimes.second.at(i).first].first.add(currentTimes.second.at(i).second);
    }

    updateTable();
  }
}

void StopwatchViewer::updateTable() {
  int currentNumTimers = 0;

  for (std::map<
           uint64_t,
           std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>>::
           const_iterator it = cache.begin();
       it != cache.end();
       it++) {
    currentNumTimers += it->second.size();
  }

  tableWidget->setRowCount(currentNumTimers);

  std::vector<std::pair<std::string, float>> plotVals;

  for (std::map<
           uint64_t,
           std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>>::
           const_iterator it = cache.begin();
       it != cache.end();
       it++) {
    const std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>&
        stopwatch = it->second;

    for (std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>::
             const_iterator it = stopwatch.begin();
         it != stopwatch.end();
         it++) {
      TableRow& newEntry = const_cast<TableRow&>(it->second.second);

      if (newEntry.isUninit()) {
        newEntry.row = lastRow++;
        newEntry.tableItems = new QTableWidgetItem[tableWidget->columnCount() - 1];
        newEntry.tableItems[0].setText(QString::fromStdString(it->first));
        newEntry.tableItems[1].setText(QString::number(it->second.first[0]));
        newEntry.tableItems[2].setText(QString::number(it->second.first.getMinimum()));
        newEntry.tableItems[3].setText(QString::number(it->second.first.getMaximum()));
        newEntry.tableItems[4].setText(QString::number(it->second.first.getAverage()));
        newEntry.tableItems[5].setText(QString::number(it->second.first.getReciprocal() * 1000.0));

        newEntry.checkItem = new QCheckBox();

        if (enabledBeforeReset.count(it->first)) {
          newEntry.checkItem->setChecked(true);
        }

        connect(newEntry.checkItem, SIGNAL(stateChanged(int)), this, SLOT(checkboxHit()));

        tableWidget->setItem(newEntry.row, 0, &newEntry.tableItems[0]);
        tableWidget->setCellWidget(newEntry.row, 1, newEntry.checkItem);
        tableWidget->setItem(newEntry.row, 2, &newEntry.tableItems[1]);
        tableWidget->setItem(newEntry.row, 3, &newEntry.tableItems[2]);
        tableWidget->setItem(newEntry.row, 4, &newEntry.tableItems[3]);
        tableWidget->setItem(newEntry.row, 5, &newEntry.tableItems[4]);
        tableWidget->setItem(newEntry.row, 6, &newEntry.tableItems[5]);
      } else {
        tableWidget->item(newEntry.row, 0)->setText(QString::fromStdString(it->first));
        tableWidget->item(newEntry.row, 2)->setText(QString::number(it->second.first[0]));
        tableWidget->item(newEntry.row, 3)->setText(QString::number(it->second.first.getMinimum()));
        tableWidget->item(newEntry.row, 4)->setText(QString::number(it->second.first.getMaximum()));
        tableWidget->item(newEntry.row, 5)->setText(QString::number(it->second.first.getAverage()));
        tableWidget->item(newEntry.row, 6)
            ->setText(QString::number(it->second.first.getReciprocal() * 1000.0));
      }

      QCheckBox* cellCheckBox = qobject_cast<QCheckBox*>(tableWidget->cellWidget(newEntry.row, 1));

      if (cellCheckBox->isChecked()) {
        plotVals.emplace_back(
            it->first,
            tableWidget->item(newEntry.row, plotChoice->currentIndex() + 2)->text().toDouble());
      }
    }
  }

  plotHolderWidget->update(plotVals);
}

void StopwatchViewer::flushCache() {
  enabledBeforeReset.clear();

  for (auto& cacheIt : cache) {
    for (auto& stopIt : cacheIt.second) {
      if (stopIt.second.second.checkItem && stopIt.second.second.checkItem->isChecked()) {
        enabledBeforeReset.insert(stopIt.first);
      }
    }
  }

  cache.clear();
  lastRow = 0;
  updateTable();
  plotHolderWidget->clear();
}
