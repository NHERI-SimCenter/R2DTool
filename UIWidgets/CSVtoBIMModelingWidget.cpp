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

#include "AssetsWidget.h"
#include "CSVReaderWriter.h"
#include "CSVtoBIMModelingWidget.h"
#include "ComponentInputWidget.h"
#include "WorkflowAppR2D.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>

CSVtoBIMModelingWidget::CSVtoBIMModelingWidget(QWidget *parent) : SimCenterAppWidget(parent)
{    

    QGridLayout* gridLayout = new QGridLayout(this);

    QLabel* IDLabel = new QLabel("Select a range of building IDs for which to create a model. By default, all buildings selected for analysis are converted into a BIM.",this);

    selectComponentsLineEdit = new QLineEdit();
    selectComponentsLineEdit->setMaximumWidth(1000);
    selectComponentsLineEdit->setMinimumWidth(400);
    selectComponentsLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    selectComponentsLineEdit->setPlaceholderText("e.g., 1, 3, 5-10, 12");

    // Create a regExp validator to make sure only '-' & ',' & ' ' & numbers are input
    QRegExp LERegExp ("((([1-9][0-9]*)|([1-9][0-9]*-[1-9][0-9]*))[ ]*,[ ]*)*([[1-9][0-9]*-[1-9][0-9]*|[1-9][0-9]*)");
    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);
    selectComponentsLineEdit->setValidator(LEValidator);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addWidget(IDLabel,0,0);
    gridLayout->addWidget(selectComponentsLineEdit,1,0);
    gridLayout->addItem(vspacer,2,0,1,2);

}


CSVtoBIMModelingWidget::~CSVtoBIMModelingWidget()
{

}


bool CSVtoBIMModelingWidget::outputToJSON(QJsonObject &jsonObj)
{

    jsonObj.insert("Application", "CSV_to_BIM");

    QJsonObject appDataObj;
    appDataObj.insert("Min", 1);
    appDataObj.insert("Max", 3);
    //appDataObj.insert("buildingSourceFile", buildingSourceFile);

    jsonObj.insert("ApplicationData",appDataObj);

    return true;
}


void CSVtoBIMModelingWidget::clear(void)
{
    selectComponentsLineEdit->clear();
}









