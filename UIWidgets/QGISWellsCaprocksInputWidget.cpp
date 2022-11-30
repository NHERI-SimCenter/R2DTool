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

// Written by: Stevan Gavrilovic

#include "QGISWellsCaprocksInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentTableView.h"
#include "ComponentDatabaseManager.h"
#include "AssetFilterDelegate.h"
#include "LineAssetInputWidget.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QPushButton>

#include <qgsfield.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgsmarkersymbol.h>

QGISWellsCaprocksInputWidget::QGISWellsCaprocksInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : PointAssetInputWidget(parent, visWidget, assetType, appType)
{
    QGISWellsCaprocksInputWidget::createComponentsBox();
}


void QGISWellsCaprocksInputWidget::createComponentsBox(void)
{
    QVBoxLayout* inputLayout = new QVBoxLayout();

    QHBoxLayout* welltraceLayout = new QHBoxLayout();

    QLabel* pathWellTraceLabel = new QLabel("Directory containing well traces:");

    QPushButton* pathWellTraceButton = new QPushButton();
    pathWellTraceButton->setText(tr("Browse"));
    pathWellTraceButton->setMaximumWidth(150);
    connect(pathWellTraceButton,&QPushButton::clicked,this,&QGISWellsCaprocksInputWidget::handleWellTracesDirDialog);

    pathWellTraceLE = new QLineEdit();

    welltraceLayout->addWidget(pathWellTraceLabel);
    welltraceLayout->addWidget(pathWellTraceLE);
    welltraceLayout->addWidget(pathWellTraceButton);

    inputLayout->addLayout(welltraceLayout);

    QHBoxLayout* caprockLayout = new QHBoxLayout();

    QLabel* pathCaprockShp = new QLabel("Caprock shapefile:");

    QPushButton* pathCaprockShpButton = new QPushButton();
    pathCaprockShpButton->setText(tr("Browse"));
    pathCaprockShpButton->setMaximumWidth(150);
    connect(pathCaprockShpButton,&QPushButton::clicked,this,&QGISWellsCaprocksInputWidget::handleCaprockDialog);

    pathCaprockShpLE = new QLineEdit();

    caprockLayout->addWidget(pathCaprockShp);
    caprockLayout->addWidget(pathCaprockShpLE);
    caprockLayout->addWidget(pathCaprockShpButton);

    inputLayout->addLayout(caprockLayout);

    auto insPoint = mainWidgetLayout->count();

    mainWidgetLayout->insertLayout(insPoint-3,inputLayout);
}



void QGISWellsCaprocksInputWidget::handleWellTracesDirDialog(void)
{
    auto newPathToInputFile = QFileDialog::getExistingDirectory(this,tr("Directory containing well traces"));

    // Return if the user cancels
    if(newPathToInputFile.isEmpty())
        return;

    pathWellTraceLE->setText(newPathToInputFile);

}


bool QGISWellsCaprocksInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    auto pathWellTraceDir = rvObject.value("WellTraceDir").toString();

    if(pathWellTraceDir.isEmpty())
    {
        this->errorMessage("Error, the required input 'WellTraceDir' is missing in "+QString(__FUNCTION__));
        return false;
    }

    QFileInfo fileInfoWT(pathWellTraceDir);

    if(!fileInfoWT.exists())
        pathWellTraceDir = QDir::currentPath() + QDir::separator() + pathWellTraceDir;

    fileInfoWT.setFile(pathWellTraceDir);

    if(!fileInfoWT.exists())
    {
        this->errorMessage("Error, could not find the well trace directory at any one of the following paths: "+rvObject.value("WellTraceDir").toString()+","+ pathWellTraceDir+" in "+QString(__FUNCTION__));
        return false;
    }


    pathWellTraceLE->setText(fileInfoWT.absoluteFilePath());

    auto pathCaprockShpFile = rvObject.value("PathToCaprockShapefile").toString();

    if(pathCaprockShpFile.isEmpty())
    {
        this->errorMessage("Error, the required input 'PathToCaprockShapefile' is missing in "+QString(__FUNCTION__));
        return false;
    }

    QFileInfo fileInfo(pathCaprockShpFile);

    if(!fileInfo.exists())
        pathCaprockShpFile = QDir::currentPath() + QDir::separator() + pathCaprockShpFile;

    fileInfo.setFile(pathCaprockShpFile);

    if(!fileInfo.exists())
    {
        this->errorMessage("Error, could not find the caprock shapefile file at any one of the following paths: "+rvObject.value("PathToCaprockShapefile").toString()+","+ pathToComponentInputFile+" in "+QString(__FUNCTION__));
        return false;
    }


    pathCaprockShpLE->setText(fileInfo.absoluteFilePath());

    this->loadCaprocksLayer();

    return PointAssetInputWidget::inputFromJSON(rvObject);
}


bool QGISWellsCaprocksInputWidget::outputToJSON(QJsonObject &rvObject)
{
   auto res = PointAssetInputWidget::outputToJSON(rvObject);

   if(!res)
   {
       this->errorMessage("Error output to json in "+QString(__FUNCTION__));
       return false;
   }


   auto pathWellTraceDir = pathWellTraceLE->text();

   if(pathWellTraceDir.isEmpty())
   {
       this->errorMessage("The path to well trace directory is empty in "+QString(__FUNCTION__));
       return false;
   }


   rvObject.insert("WellTraceDir",pathWellTraceDir);


   auto pathCaprockShp = pathCaprockShpLE->text();

   if(pathCaprockShp.isEmpty())
   {
       this->errorMessage("The path to caprock shapefile is empty in "+QString(__FUNCTION__));
       return false;
   }

   rvObject.insert("PathToCaprockShapefile",pathCaprockShp);

   return true;
}


void QGISWellsCaprocksInputWidget::handleCaprockDialog(void)
{
    auto newPathToInputFile = QFileDialog::getOpenFileName(this,tr("Caprock shapefile"));

    // Return if the user cancels
    if(newPathToInputFile.isEmpty())
        return;

    pathCaprockShpLE->setText(newPathToInputFile);

    this->loadCaprocksLayer();
}


void QGISWellsCaprocksInputWidget::clear()
{    
    caprocksLayer = nullptr;
    pathCaprockShpLE->clear();
    pathWellTraceLE->clear();
    PointAssetInputWidget::clear();
}


void QGISWellsCaprocksInputWidget::loadCaprocksLayer()
{
    auto pathToCaprockFile = pathCaprockShpLE->text();

    if(!QFileInfo::exists(pathToCaprockFile))
    {
        this->errorMessage("Error, the caprock file :"+pathToCaprockFile+ "does not exist");
    }

    if(caprocksLayer)
        theVisualizationWidget->removeLayer(caprocksLayer);

    caprocksLayer = theVisualizationWidget->addVectorLayer(pathToCaprockFile,"Caprocks","ogr");

    if(!caprocksLayer)
    {
        this->errorMessage("Error creating the caprock layer in "+QString(__FUNCTION__));
    }
}


