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

#include "HazardsWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"
#include "EarthquakeInputWidget.h"
#include "VisualizationWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

HazardsWidget::HazardsWidget(QWidget *parent, VisualizationWidget* visWidget, RandomVariablesContainer* RVContainer) : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer), theVisualizationWidget(visWidget)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Hazards"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();
    mainLayout->addWidget(theComponentSelection);

    theComponentSelection->setMaxWidth(120);

    earthquakesWidget = std::make_unique<EarthquakeInputWidget>(this, theVisualizationWidget, theRandomVariablesContainer);

    QWidget* earthquakeBox = earthquakesWidget->getEarthquakesWidget();
    QGroupBox* windBox = this->getWindBox();
    QGroupBox* groundSetBox = this->getGroundSettlementBox();
    QGroupBox* LandslideBox = this->getLandSlideBox();

    theComponentSelection->addComponent("Earthquakes",earthquakeBox);
    theComponentSelection->addComponent("Wind",windBox);
    theComponentSelection->addComponent("Landslides",LandslideBox);
    theComponentSelection->addComponent("Ground\nSettlement",groundSetBox);

    theComponentSelection->displayComponent("Earthquakes");

    this->setLayout(mainLayout);
}

HazardsWidget::~HazardsWidget()
{

}


bool HazardsWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool HazardsWidget::inputFromJSON(QJsonObject &jsonObject)
{

    return false;
}


bool HazardsWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool HazardsWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return true;
}

bool HazardsWidget::copyFiles(QString &destDir)
{

    return false;
}

QGroupBox* HazardsWidget::getLandSlideBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Landslides");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* HazardsWidget::getWindBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Wind");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* HazardsWidget::getGroundSettlementBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Ground Settlement");
    groupBox->setFlat(true);


    return groupBox;
}


