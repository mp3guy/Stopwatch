#include "stopwatchviewer.h"

StopwatchViewer::StopwatchViewer(QWidget * parent)
 : QWidget(parent)
{
	ui.setupUi(this);

	plotHolderWidget = new PlotHolderWidget();

	mySocket = new QUdpSocket(this);
	mySocket->bind(45454, QUdpSocket::ShareAddress);
    connect(mySocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));

	ui.tableWidget->setColumnCount(NUM_FIELDS);

	QStringList columnTitles;
	columnTitles << "Item" << "Plot" << "Last (ms)" << "Min (ms)" << "Max (ms)" << "Avg (ms)" << "Hz";

	assert(columnTitles.length() == NUM_FIELDS);

	ui.tableWidget->setHorizontalHeaderLabels(columnTitles);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	columnTitles.removeFirst();
	columnTitles.removeFirst();

	ui.plotChoiceComboBox->addItems(columnTitles);

    connect(ui.flushButton, SIGNAL(clicked()), this, SLOT(flushCache()));
    connect(ui.plotButton, SIGNAL(clicked()), this, SLOT(plotClicked()));

    lastRow = 0;
}

StopwatchViewer::~StopwatchViewer()
{
    mySocket->close();

    delete plotHolderWidget;

    delete mySocket;
}

void StopwatchViewer::checkboxHit()
{
    int numChecked = 0;

    for(int i = 0; i < ui.tableWidget->rowCount(); i++)
    {
        QCheckBox * cellCheckBox = qobject_cast<QCheckBox *>(ui.tableWidget->cellWidget(i, 1));

        if(cellCheckBox->isChecked())
        {
            numChecked++;
        }

        if(numChecked == DataPlotWidget::NUM_PLOTS)
        {
            for(int j = 0; j < ui.tableWidget->rowCount(); j++)
            {
                QCheckBox * checkBox = qobject_cast<QCheckBox *>(ui.tableWidget->cellWidget(j, 1));

                if(!checkBox->isChecked())
                {
                    checkBox->setCheckable(false);
                }
            }
            break;
        }
    }

    if(numChecked < DataPlotWidget::NUM_PLOTS)
    {
        for(int j = 0; j < ui.tableWidget->rowCount(); j++)
        {
            QCheckBox * checkBox = qobject_cast<QCheckBox *>(ui.tableWidget->cellWidget(j, 1));
            checkBox->setCheckable(true);
        }
    }
}

void StopwatchViewer::processPendingDatagram()
{
    QByteArray datagram;
    datagram.resize(mySocket->pendingDatagramSize());
    mySocket->readDatagram(datagram.data(), datagram.size());

    int * data = (int *)datagram.data();

    if(datagram.size() > 0 &&
       datagram.size() == data[0])
    {
        std::pair<unsigned long long int, std::vector<std::pair<std::string, float> > > currentTimes = StopwatchDecoder::decodePacket((unsigned char *)datagram.data(), datagram.size());

        std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> > & stopwatch = cache[currentTimes.first];

        for(unsigned int i = 0; i < currentTimes.second.size(); i++)
        {
            stopwatch[currentTimes.second.at(i).first].first.add(currentTimes.second.at(i).second);
        }

        updateTable();
    }
}

void StopwatchViewer::updateTable()
{
    int currentNumTimers = 0;

    for(std::map<unsigned long long int, std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> > >::const_iterator it = cache.begin(); it != cache.end(); it++)
    {
        currentNumTimers += it->second.size();
    }

    ui.tableWidget->setRowCount(currentNumTimers);

    std::vector<float> plotVals;

    for(std::map<unsigned long long int, std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> > >::const_iterator it = cache.begin(); it != cache.end(); it++)
    {
        const std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> > & stopwatch = it->second;

        for(std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> >::const_iterator it = stopwatch.begin(); it != stopwatch.end(); it++)
        {
            TableRow & newEntry = const_cast<TableRow&>(it->second.second);

            if(newEntry.isUninit())
            {
                newEntry.row = lastRow++;
                newEntry.tableItems = new QTableWidgetItem[NUM_FIELDS - 1];
                newEntry.tableItems[0].setText(QString::fromStdString(it->first));
                newEntry.tableItems[1].setText(QString::number(it->second.first[0]));
                newEntry.tableItems[2].setText(QString::number(it->second.first.getMinimum()));
                newEntry.tableItems[3].setText(QString::number(it->second.first.getMaximum()));
                newEntry.tableItems[4].setText(QString::number(it->second.first.getAverage()));
                newEntry.tableItems[5].setText(QString::number(it->second.first.getReciprocal() * 1000.0));

                newEntry.checkItem = new QCheckBox();
                connect(newEntry.checkItem, SIGNAL(stateChanged(int)), this, SLOT(checkboxHit()));

                ui.tableWidget->setItem(newEntry.row, 0, &newEntry.tableItems[0]);
                ui.tableWidget->setCellWidget(newEntry.row, 1, newEntry.checkItem);
                ui.tableWidget->setItem(newEntry.row, 2, &newEntry.tableItems[1]);
                ui.tableWidget->setItem(newEntry.row, 3, &newEntry.tableItems[2]);
                ui.tableWidget->setItem(newEntry.row, 4, &newEntry.tableItems[3]);
                ui.tableWidget->setItem(newEntry.row, 5, &newEntry.tableItems[4]);
                ui.tableWidget->setItem(newEntry.row, 6, &newEntry.tableItems[5]);
            }
            else
            {
                ui.tableWidget->item(newEntry.row, 0)->setText(QString::fromStdString(it->first));
                ui.tableWidget->item(newEntry.row, 2)->setText(QString::number(it->second.first[0]));
                ui.tableWidget->item(newEntry.row, 3)->setText(QString::number(it->second.first.getMinimum()));
                ui.tableWidget->item(newEntry.row, 4)->setText(QString::number(it->second.first.getMaximum()));
                ui.tableWidget->item(newEntry.row, 5)->setText(QString::number(it->second.first.getAverage()));
                ui.tableWidget->item(newEntry.row, 6)->setText(QString::number(it->second.first.getReciprocal() * 1000.0));
            }

            QCheckBox * cellCheckBox = qobject_cast<QCheckBox *>(ui.tableWidget->cellWidget(newEntry.row, 1));

            if(cellCheckBox->isChecked())
            {
                plotVals.push_back(ui.tableWidget->item(newEntry.row, ui.plotChoiceComboBox->currentIndex() + 2)->text().toDouble());
            }
        }
    }

    ui.tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);

    bool enabled[DataPlotWidget::NUM_PLOTS];

    for (size_t i = 0; i < DataPlotWidget::NUM_PLOTS; i++)
    {
        enabled[i] = plotVals.size() > i;
    }


    float values[DataPlotWidget::NUM_PLOTS] = {0};

    for(int i = 0; i < (int)plotVals.size() && i < DataPlotWidget::NUM_PLOTS; i++)
    {
        values[i] = plotVals[i];
    }

    plotHolderWidget->update(&values[0], &enabled[0]);
}

void StopwatchViewer::flushCache()
{
    cache.clear();
    lastRow = 0;
    updateTable();
}
