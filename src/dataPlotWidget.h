/*
 * @author fiachra & tom
 */

#ifndef DATA_PLOT_WIDGET_H
#define DATA_PLOT_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QPushButton>
#include <QPainter>
#include <iostream>

class QPaintEvent;

class DataPlotWidget : public QWidget
{
    Q_OBJECT;

    public:
        DataPlotWidget(QWidget * parent = 0);
        virtual ~DataPlotWidget();

        void updatePlot(float * value, bool * enabled);
        void resetPlot();
        void setDataLength(int length);

        static const int DEFAULT_DATA_LENGTH = 100;
        static const int NUM_PLOTS = 4;
        static const int PLOT_WIDTH = 640;
        static const int PLOT_HEIGHT = 240;
        static const int PEN_WIDTH = 1;
        static const int PEN_HIGHLIGHT_WIDTH = 2;
    private:
        QPainter painter;

        void paintEvent(QPaintEvent * event);
        int getPlotY(float val, float dataMin, float dataMax);
        void drawDataPlot();
        
        int lastLimit;
        int dataLength;
        float ** dataArray;
        int currentCount;
        int currentIndex;
};

#endif

