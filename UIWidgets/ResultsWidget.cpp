/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Stevan Gavrilovic
// Latest revision: 10.01.2020

#include "ResultsWidget.h"
#include "sectiontitle.h"

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>

using namespace Esri::ArcGISRuntime;

ResultsWidget::ResultsWidget(QWidget *parent)
    : SimCenterWidget(parent), resultWidget(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Results"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    QHBoxLayout *theVizLayout = new QHBoxLayout();

    auto visSelectBox = this->getVisSelectionGroupBox();

    theVizLayout->addWidget(visSelectBox);

    // Create the Widget view
    mapViewWidget = new MapGraphicsView(this);

    // Create a map using the topographic Basemap
    mapObject = new Map(Basemap::topographic(this), this);

    mapViewWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // Set map to map view
    mapViewWidget->setMap(mapObject);

    theVizLayout->addWidget(mapViewWidget);

    // Export objects
    QHBoxLayout *theExportLayout = new QHBoxLayout();

    QLabel* exportLabel = new QLabel("Export folder:");

    auto exportPathLineEdit = new QLineEdit();
    exportPathLineEdit->setMaximumWidth(1000);
    exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *exportBrowseFileButton = new QPushButton();
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportFileButton = new QPushButton();
    exportFileButton->setText(tr("Export to CSV"));
    exportFileButton->setMaximumWidth(150);

    theExportLayout->addStretch();
    theExportLayout->addWidget(exportLabel);
    theExportLayout->addWidget(exportPathLineEdit);
    theExportLayout->addWidget(exportBrowseFileButton);
    theExportLayout->addWidget(exportFileButton);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(theVizLayout);
    mainLayout->addLayout(theExportLayout);

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
}


ResultsWidget::~ResultsWidget()
{

}


bool ResultsWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool ResultsWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return true;
}


int ResultsWidget::processResults(QString &filenameResults, QString &filenameTab) {

    return 0;
}


QGroupBox* ResultsWidget::getVisSelectionGroupBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Select Data to Visualize");
    groupBox->setMaximumWidth(250);
//    groupBox->setStyleSheet("background-color: white;");

    auto layout = new QVBoxLayout();
    groupBox->setLayout(layout);

    auto mapDataLabel = new QLabel("Pre-packaged maps and data sets:");
    mapDataLabel->setStyleSheet("font-weight: bold; color: black");

    QCheckBox* CGS1Checkbox = new QCheckBox("CGS Geologic Map (Ref.)");
    QCheckBox* CGS2Checkbox = new QCheckBox("CGS Liquefaction Susceptibility\nMap (Ref.)");
    CGS1Checkbox->setChecked(true);

    auto DVLabel = new QLabel("Decision Variables:");
    DVLabel->setStyleSheet("font-weight: bold; color: black");

    auto allRepairsLabel = new QLabel("Annual number of repairs \n(breaks and leaks)");
    allRepairsLabel->setStyleSheet("text-decoration: underline; color: black");

    QCheckBox* GMCheckbox = new QCheckBox("Ground Motion");
    QCheckBox* LatSpreadCheckbox = new QCheckBox("Lateral Spreading");
    QCheckBox* LandslideCheckbox = new QCheckBox("Landslide");
    GMCheckbox->setChecked(true);

    auto breaksLabel = new QLabel("Annual number of breaks");
    breaksLabel->setStyleSheet("text-decoration: underline; color: black");
    QCheckBox* GMCheckbox2 = new QCheckBox("Ground Motion");
    QCheckBox* LatSpreadCheckbox2 = new QCheckBox("Lateral Spreading");
    QCheckBox* LandslideCheckbox2 = new QCheckBox("Landslide");

    auto othersLabel = new QLabel("Others:");
    othersLabel->setStyleSheet("font-weight: bold; color: black");
    QCheckBox* shakeMapCheckbox = new QCheckBox("ShakeMap");

    layout->addWidget(mapDataLabel);
    layout->addWidget(CGS1Checkbox);
    layout->addWidget(CGS2Checkbox);
    layout->addWidget(DVLabel);
    layout->addWidget(allRepairsLabel);
    layout->addWidget(GMCheckbox);
    layout->addWidget(LatSpreadCheckbox);
    layout->addWidget(LandslideCheckbox);
    layout->addWidget(breaksLabel);
    layout->addWidget(GMCheckbox2);
    layout->addWidget(LatSpreadCheckbox2);
    layout->addWidget(LandslideCheckbox2);
    layout->addWidget(othersLabel);
    layout->addWidget(shakeMapCheckbox);

    layout->addStretch();

    return groupBox;
}

