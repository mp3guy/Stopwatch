#include "ViewerWidget.h"

ViewerWidget::ViewerWidget() {
  this->resize(1024, 768);
  this->setWindowTitle("StopwatchViewer");
  this->setObjectName("StopwatchViewerUi");

  auto topWidget = new QWidget(this);
  auto topLayout = new QVBoxLayout(this);
  tableWidget_ = new QTableWidget(this);
  tableWidget_->setVisible(false);
  tableView_ = new QTableView(this);
  QFont font("Monospace");
  font.setStyleHint(QFont::TypeWriter);
  tableView_->setFont(font);
  flushButton = new QPushButton("Flush Cache");

  auto plotAll = new QPushButton("Plot All");
  auto plotFiltered = new QPushButton("Plot Filtered");
  auto plotRemoveFiltered = new QPushButton("Remove Filtered");
  auto plotRemoveAll = new QPushButton("Remove All");
  filterText_ = new QLineEdit("");
  filterText_->setPlaceholderText("Filter Timing Items");
  auto topTopLayout = new QHBoxLayout();
  topTopLayout->addWidget(flushButton);
  topTopLayout->addWidget(plotAll);
  topTopLayout->addWidget(plotFiltered);
  topTopLayout->addWidget(plotRemoveFiltered);
  topTopLayout->addWidget(plotRemoveAll);
  topLayout->addLayout(topTopLayout);
  topLayout->addWidget(filterText_);

  topLayout->addWidget(tableWidget_);
  topLayout->addWidget(tableView_);
  topWidget->setLayout(topLayout);

  auto bottomWidget = new QWidget(this);
  auto bottomLayout = new QVBoxLayout();
  auto vbox = new QVBoxLayout();
  auto hbox = new QHBoxLayout();
  plotChoice_ = new QComboBox(this);
  plotHolderWidget_ = new PlotHolderWidget(this);

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

  QStringList columnTitles;
  columnTitles << "Item"
               << "Plot"
               << "  Last (ms)"
               << "   Min (ms)"
               << "   Max (ms)"
               << "   Avg (ms)"
               << "       Hz";

  tableWidget_->setColumnCount(columnTitles.length());
  tableWidget_->setHorizontalHeaderLabels(columnTitles);
  tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  columnTitles.removeFirst();
  columnTitles.removeFirst();

  plotChoice_->addItems(columnTitles);

  proxy_ = new QSortFilterProxyModel(this);
  proxy_->setSourceModel(tableWidget_->model());
  proxy_->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);

  tableView_->setModel(proxy_);
  tableView_->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
  tableView_->setSortingEnabled(true);

  tableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setTextElideMode(Qt::TextElideMode::ElideRight);
  tableView_->horizontalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
  tableView_->setTextElideMode(Qt::TextElideMode::ElideLeft);
  tableView_->verticalHeader()->setVisible(false);

  connect(filterText_, &QLineEdit::textChanged, this, [&](auto& text) {
    proxy_->setFilterFixedString(filterText_->text());
  });

  // enabled/disable all plots that match the text filter
  const auto setAllChecked = [&](const bool checked, const bool matchFilter) {
    const auto qtChecked = checked ? Qt::Checked : Qt::Unchecked;
    if (!matchFilter) {
      for (int i = 0; i < tableWidget_->rowCount(); i++) {
        tableWidget_->item(i, 1)->setCheckState(qtChecked);
      }
    } else {
      for (int i = 0; i < proxy_->rowCount(); i++) {
        auto item = proxy_->mapToSource(proxy_->index(i, 0));
        tableWidget_->item(item.row(), 1)->setCheckState(qtChecked);
      }
    }
  };
  connect(plotAll, &QPushButton::clicked, this, [=](bool checked) { setAllChecked(true, false); });

  connect(
      plotFiltered, &QPushButton::clicked, this, [=](bool checked) { setAllChecked(true, true); });

  connect(plotRemoveFiltered, &QPushButton::clicked, this, [=](bool checked) {
    setAllChecked(false, true);
  });
  connect(plotRemoveAll, &QPushButton::clicked, this, [=](bool checked) {
    setAllChecked(false, false);
  });
}

void ViewerWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    QApplication::quit();
  }
}

void ViewerWidget::updateTable(
    const std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>&
        signatureToNameToDurations,
    const std::unordered_set<std::string>& enabledBeforeReset,
    int& lastRow) {
  int currentNumTimers = 0;

  for (auto it = signatureToNameToDurations.begin(); it != signatureToNameToDurations.end(); it++) {
    currentNumTimers += it->second.size();
  }

  tableWidget_->setRowCount(currentNumTimers);

  std::vector<std::pair<std::string, float>> plotVals;

  for (auto it = signatureToNameToDurations.begin(); it != signatureToNameToDurations.end(); it++) {
    const auto& stopwatch = it->second;

    for (auto it = stopwatch.begin(); it != stopwatch.end(); it++) {
      TableRow& newEntry = const_cast<TableRow&>(it->second.second);

      if (newEntry.isUninit()) {
        newEntry.row = lastRow++;

        while ((int)newEntry.tableItems.size() < tableWidget_->columnCount()) {
          newEntry.tableItems.emplace_back(new QTableWidgetItem());
        }

        newEntry.tableItems[0]->setText(QString::fromStdString(it->first));
        newEntry.tableItems[1]->setCheckState(Qt::Unchecked);
        newEntry.tableItems[2]->setData(
            Qt::DisplayRole, QString::number(it->second.first[0], 'f', 3).toFloat());

        newEntry.tableItems[3]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getMinimum(), 'f', 3).toFloat());

        newEntry.tableItems[4]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getMaximum(), 'f', 3).toFloat());
        newEntry.tableItems[5]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getAverage(), 'f', 3).toFloat());
        newEntry.tableItems[6]->setData(
            Qt::DisplayRole,

            QString::number(it->second.first.getReciprocal() * 1000.0, 'f', 3).toFloat());

        if (enabledBeforeReset.count(it->first)) {
          newEntry.tableItems[1]->setCheckState(Qt::Checked);
        }

        tableWidget_->setItem(newEntry.row, 0, newEntry.tableItems[0]);
        tableWidget_->setItem(newEntry.row, 1, newEntry.tableItems[1]);
        tableWidget_->setItem(newEntry.row, 2, newEntry.tableItems[2]);
        tableWidget_->setItem(newEntry.row, 3, newEntry.tableItems[3]);
        tableWidget_->setItem(newEntry.row, 4, newEntry.tableItems[4]);
        tableWidget_->setItem(newEntry.row, 5, newEntry.tableItems[5]);
        tableWidget_->setItem(newEntry.row, 6, newEntry.tableItems[6]);

        for (int i = 0; i < 7; i++) {
          if (i != 1) {
            tableWidget_->item(newEntry.row, i)
                ->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
          }
        }

      } else {
        newEntry.tableItems[0]->setText(QString::fromStdString(it->first));
        newEntry.tableItems[2]->setData(
            Qt::DisplayRole, QString::number(it->second.first[0], 'f', 3).toFloat());

        newEntry.tableItems[3]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getMinimum(), 'f', 3).toFloat());

        newEntry.tableItems[4]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getMaximum(), 'f', 3).toFloat());
        newEntry.tableItems[5]->setData(
            Qt::DisplayRole, QString::number(it->second.first.getAverage(), 'f', 3).toFloat());
        newEntry.tableItems[6]->setData(
            Qt::DisplayRole,
            QString::number(it->second.first.getReciprocal() * 1000.0, 'f', 3).toFloat());
      }

      if (newEntry.tableItems[1]->checkState() == Qt::Checked) {
        plotVals.emplace_back(
            it->first,
            tableWidget_->item(newEntry.row, plotChoice_->currentIndex() + 2)->text().toDouble());
      }
    }
  }

  plotHolderWidget_->update(plotVals);
}
