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

// Written: fmckenna

#include "UserDefinedEDPR.h"
#include "EDP.h"
#include <sectiontitle.h>

#include <QPushButton>
#include <QFileDialog>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QGridLayout>

#include <string>
#include <fstream>

using namespace std;

UserDefinedEDPR::UserDefinedEDPR(RandomVariablesContainer *theRandomVariableIW, QWidget *parent) : SimCenterAppWidget(parent), theRandomVariablesContainer(theRandomVariableIW)
{

    QVBoxLayout *layout = new QVBoxLayout();
    QGridLayout *scriptLayout = new QGridLayout();

    // add stuff to enter processing script
    QLabel *labelPS = new QLabel("EDP Specs");
    EDPSpecsLineEdit = new QLineEdit;
    QPushButton *chooseProcessingScriptButton = new QPushButton();
    chooseProcessingScriptButton->setText(tr("Choose"));
    connect(chooseProcessingScriptButton,SIGNAL(clicked()),this,SLOT(chooseEDPSpecs()));
    scriptLayout->addWidget(labelPS,1,0);
    scriptLayout->addWidget(EDPSpecsLineEdit,1,2);
    scriptLayout->addWidget(chooseProcessingScriptButton, 1, 4);

    QSpacerItem *spacer3 = new QSpacerItem(30,10);
    QSpacerItem *spacer4 = new QSpacerItem(10,10);
    scriptLayout->addItem(spacer3,0,1);
    scriptLayout->addItem(spacer4,0,3);
    scriptLayout->setColumnStretch(5,1);
    scriptLayout->setColumnStretch(2,1);

    scriptLayout->setHorizontalSpacing(5);
    scriptLayout->setVerticalSpacing(5);

    layout->addLayout(scriptLayout);
    QSpacerItem *spacer = new QSpacerItem(20,10);
    layout->addSpacerItem(spacer);

    layout->setSpacing(0);
    layout->setMargin(0);

    this->setLayout(layout);
}


UserDefinedEDPR::~UserDefinedEDPR()
{

}


void UserDefinedEDPR::clear(void)
{
    EDPSpecsLineEdit->setText("");
}


bool UserDefinedEDPR::outputToJSON(QJsonObject &jsonObject)
{

    jsonObject.insert("Application","UserDefinedEDP_R");

    QJsonObject appData;

    appData.insert("EDPspecs",EDPSpecsLineEdit->text());

    jsonObject.insert("ApplicationData",appData);

    return true;
}


bool UserDefinedEDPR::inputFromJSON(QJsonObject &jsonObject)
{


    return true;
}


void UserDefinedEDPR::chooseEDPSpecs(void)
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "All files (*.*)");
    this->setEDPSpecsFile(fileName);
}


void UserDefinedEDPR::setEDPSpecsFile(const QString& filename)
{
    if (FILE *file = fopen(filename.toStdString().c_str(), "r"))
    {
        EDPSpecsLineEdit->setText(filename);
        fclose(file);
        return;
    }
    else
    {
        QString msg = "Error could not find the file: "+filename;
        this->userMessageDialog(msg);
    }

}


bool UserDefinedEDPR::copyFiles(QString &dirName)
{
     QString filename = EDPSpecsLineEdit->text();
     if (!filename.isEmpty()) {

         if (this->copyFile(filename, dirName) ==  false) {
             emit sendErrorMessage(QString("ERROR: copyFiles: failed to copy") + filename);
             return false;
         }
     }


     return 0;
 }


