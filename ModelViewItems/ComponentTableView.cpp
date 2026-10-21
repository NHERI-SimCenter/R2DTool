/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "ComponentTableModel.h"
#include "ComponentTableView.h"
#include "VisualizationWidget.h"

#include <QDebug>
#include <QMenu>
#include <QVariant>
#include <QHeaderView>
#include <QDialog>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>
#include <QtGlobal>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <algorithm>
#include <cmath>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

ComponentTableView::ComponentTableView(QWidget *parent, bool showStatistics) : QTableView(parent)
{
    tableModel = new ComponentTableModel();
    this->setModel(tableModel);

    this->hide();
    this->setToolTip("Component details");
    this->verticalHeader()->setVisible(false);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    this->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    this->setEditTriggers(EditTrigger::DoubleClicked);
    this->setSelectionMode(SelectionMode::SingleSelection);

    if(showStatistics)
    {
        this->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &ComponentTableView::handleHeaderContextMenu);
    }
}


void ComponentTableView::handleHeaderContextMenu(const QPoint& pos)
{
    auto col = this->horizontalHeader()->logicalIndexAt(pos);

    if(col < 0)
        return;

    QMenu menu;
    QAction* statsAction = menu.addAction("Show Statistics for "+this->horizontalHeaderItem(col));

    if(menu.exec(this->horizontalHeader()->mapToGlobal(pos)) == statsAction)
        this->showColumnStatistics(col);
}


void ComponentTableView::showColumnStatistics(int col)
{
    auto heading = this->horizontalHeaderItem(col);
    auto& data = tableModel->getTableData();

    // Collect the non-empty values, the column is numeric if all of them are numbers
    QStringList values;
    values.reserve(data.size());
    int numEmpty = 0;
    for(auto&& row : data)
    {
        QString value = col < row.size() ? row.at(col).trimmed() : QString();
        if(value.isEmpty())
            ++numEmpty;
        else
            values.push_back(value);
    }

    if(values.isEmpty())
    {
        qDebug() << "ComponentTableView: no values in column " << heading;
        return;
    }

    bool isNumeric = true;
    bool isFloat = false;
    std::vector<double> numbers;
    numbers.reserve(values.size());
    for(auto&& value : values)
    {
        bool OK = false;
        double number = value.toDouble(&OK);
        if(!OK || !std::isfinite(number))
        {
            isNumeric = false;
            break;
        }
        if(number != std::floor(number))
            isFloat = true;
        numbers.push_back(number);
    }

    QString statsText = "Count: " + QString::number(values.size());
    if(numEmpty > 0)
        statsText += "   (empty: " + QString::number(numEmpty) + ")";

    auto chart = new QChart();
    chart->legend()->hide();
    chart->setTitle(heading);

    auto axisY = new QValueAxis();
    axisY->setTitleText("Frequency");
    axisY->setLabelFormat("%d");

    if(isNumeric)
    {
        auto minmax = std::minmax_element(numbers.begin(), numbers.end());
        double min = *minmax.first;
        double max = *minmax.second;

        statsText += "\nMin: " + QString::number(min) + "   Max: " + QString::number(max);

        if(isFloat)
        {
            // Welford, stable for large columns
            double mean = 0.0;
            double m2 = 0.0;
            int n = 0;
            for(auto&& x : numbers)
            {
                ++n;
                double delta = x - mean;
                mean += delta/n;
                m2 += delta*(x - mean);
            }
            double stdDev = n > 1 ? std::sqrt(m2/(n-1)) : 0.0;

            statsText += "\nMean: " + QString::number(mean) + "   Std Dev: " + QString::number(stdDev);
        }

        // Integers with a small range get one bin per value, otherwise sqrt(n) bins between 10 and 30
        int numBins = 0;
        double binStart = min;
        double binWidth = 0.0;
        if(!isFloat && max - min < 30)
        {
            numBins = static_cast<int>(max - min) + 1;
            binStart = min - 0.5;
            binWidth = 1.0;
        }
        else
        {
            numBins = std::min(30, std::max(10, static_cast<int>(std::ceil(std::sqrt(numbers.size())))));
            binWidth = (max - min)/numBins;
            if(binWidth == 0.0)
            {
                numBins = 1;
                binWidth = std::max(std::abs(min)*0.1, 1.0);
                binStart = min - binWidth/2;
            }
        }

        std::vector<int> counts(numBins, 0);
        for(auto&& x : numbers)
        {
            int bin = static_cast<int>(std::floor((x - binStart)/binWidth));
            bin = std::max(0, std::min(numBins-1, bin));
            ++counts[bin];
        }

        auto series = new QLineSeries();
        int maxCount = 0;
        for(int i = 0; i<numBins; ++i)
        {
            double left = binStart + i*binWidth;
            double right = left + binWidth;
            series->append(left, 0);
            series->append(left, counts[i]);
            series->append(right, counts[i]);
            series->append(right, 0);
            maxCount = std::max(maxCount, counts[i]);
        }

        chart->addSeries(series);

        auto axisX = new QValueAxis();
        axisX->setTitleText(heading);
        axisX->setRange(binStart, binStart + numBins*binWidth);
        if(!isFloat)
            axisX->setLabelFormat("%d");

        axisY->setRange(0, maxCount);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    else
    {
        // Text, count each value and show the most common ones
        QMap<QString, int> countMap;
        for(auto&& value : values)
            ++countMap[value];

        QVector<QPair<QString, int>> counts;
        for(auto it = countMap.cbegin(); it != countMap.cend(); ++it)
            counts.push_back(qMakePair(it.key(), it.value()));

        std::stable_sort(counts.begin(), counts.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b){ return a.second > b.second; });

        statsText += "\nDistinct values: " + QString::number(counts.size());

        const int maxBars = 30;
        int otherCount = 0;
        if(counts.size() > maxBars)
        {
            for(int i = maxBars; i<counts.size(); ++i)
                otherCount += counts[i].second;

            statsText += "   (top " + QString::number(maxBars) + " shown)";
            counts.resize(maxBars);
        }

        auto barSet = new QBarSet(heading);
        QStringList categories;
        int maxCount = otherCount;
        for(auto&& it : counts)
        {
            *barSet << it.second;
            categories << it.first;
            maxCount = std::max(maxCount, it.second);
        }
        if(otherCount > 0)
        {
            *barSet << otherCount;
            categories << "Other";
        }

        auto series = new QBarSeries();
        series->append(barSet);
        chart->addSeries(series);

        auto axisX = new QBarCategoryAxis();
        axisX->append(categories);
        if(categories.size() > 10)
            axisX->setLabelsAngle(-90);

        axisY->setRange(0, maxCount);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    auto chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    auto statsLabel = new QLabel(statsText);
    statsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(heading + " Statistics");

    auto layout = new QVBoxLayout(dialog);
    layout->addWidget(statsLabel);
    layout->addWidget(chartView);

    dialog->resize(600, 450);
    dialog->show();
}

void ComponentTableView::clear(void)
{
    tableModel->clear();
}

void ComponentTableView::setComponenetTableModel(ComponentTableModel* compTableModel)
{
    tableModel = compTableModel;
    this->setModel(tableModel);

}


int ComponentTableView::columnCount(void)
{
    return tableModel->columnCount();
}


int ComponentTableView::rowCount(void)
{
    return tableModel->rowCount();
}


QString ComponentTableView::horizontalHeaderItem(int section)
{
    return this->horizontalHeaderItemVariant(section).toString();
}


QVariant ComponentTableView::horizontalHeaderItemVariant(int section)
{
    auto headerData = tableModel->headerData(section, Qt::Horizontal);

    return headerData;
}


ComponentTableModel *ComponentTableView::getTableModel() const
{
    return tableModel;
}


QVariant ComponentTableView::item(int row, int col)
{
    return tableModel->item(row,col);
}
