#ifndef STOPWATCHVIEWER_H
#define STOPWATCHVIEWER_H

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtNetwork/QUdpSocket>
#include <iostream>
#include <map>
#include <utility>
#include <assert.h>

#include "RingBuffer.h"
#include "StopwatchDecoder.h"
#include "plotHolderWidget.h"
#include "ui_stopwatchviewer.h"

using namespace std;

class StopwatchViewer : public QWidget
{
    Q_OBJECT

    public:
        StopwatchViewer(QWidget * parent = 0);
        ~StopwatchViewer();

    private:
        static const int NUM_FIELDS = 7;

        Ui::StopwatchViewerClass ui;

        QUdpSocket * mySocket;

        bool plotAverages;

        class TableRow
        {
            public:
                TableRow()
                 : row(0), tableItems(0), checkItem(0)
                {}

                virtual ~TableRow()
                {
                    if(tableItems != 0)
                    {
                        delete [] tableItems;
                    }

                    if(checkItem != 0)
                    {
                        delete checkItem;
                    }
                }

                inline bool isUninit() const
                {
                    return tableItems == 0 || checkItem == 0;
                }

                int row;
                QTableWidgetItem * tableItems;
                QCheckBox * checkItem;
        };

        PlotHolderWidget * plotHolderWidget;

        std::map<unsigned long long int, std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow> > > cache;

        int lastRow;

        void updateTable();

    private slots:
        void processPendingDatagram();
        void flushCache();
        void checkboxHit();

        void plotClicked()
        {
            if(plotHolderWidget->isVisible())
            {
                plotHolderWidget->hide();
            }
            else
            {
                plotHolderWidget->show();
            }
        }
};

#endif // STOPWATCHVIEWER_H
