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

#include "GISMapWidget.h"
#include "VisualizationWidget.h"
#include "CustomListWidget.h"
#include "TreeItem.h"
#include "WorkflowAppOpenSRA.h"
#include "RandomVariablesWidget.h"
#include "MixedDelegate.h"

#include <QDirIterator>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QJsonArray>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QSplitter>
#include <QStandardPaths>
#include <QVBoxLayout>


GISMapWidget::GISMapWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    pathToGISMapDirectory = "";

    auto mainLayout = new QVBoxLayout(this);
    auto mainWidget = this->getMainWidget();

    mainLayout->addWidget(mainWidget);

    auto gisMapsDelegate = WorkflowAppOpenSRA::getInstance()->getTheRandomVariableWidget()->getGisMapsComboDelegate();

    assert(gisMapsDelegate);

    connect(this, &GISMapWidget::GISMapsListChanged, gisMapsDelegate, &MixedDelegate::updateComboBoxValues);

}


GISMapWidget::~GISMapWidget()
{
}


QWidget* GISMapWidget::getMainWidget(void)
{
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    listWidget = new CustomListWidget("List of Imported GIS Maps");

    splitter->addWidget(this->getGISInputWidget());
    splitter->addWidget(listWidget);

    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return splitter;
}


QWidget* GISMapWidget::getGISInputWidget(void)
{
    auto mainWidget = new QWidget();
    mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLabel* selectComponentsText = new QLabel("To import GIS files, please place them in the following folder:");
    selectComponentsText->setWordWrap(true);

    GISMapDirectoryLineEdit = new QLineEdit();
    // GISMapDirectoryLineEdit->setMinimumWidth(400);
    // GISMapDirectoryLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    QPushButton *loadButton = new QPushButton();
    loadButton->setText(tr("Load"));
    loadButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseGISMapDirectoryDialog()));

    connect(loadButton,SIGNAL(clicked()),this,SLOT(loadGISMapData()));

    QLabel* GISMapText1 = new QLabel("Files must be in a valid GIS format (*.geojson, *.gpkg, *.tif, or *.shp) and in the WGS 84 (EPSG:4326) coordinate reference system.");
    GISMapText1->setWordWrap(true);
    QLabel* GISMapText3 = new QLabel("The list of loaded GIS maps will appear on the right.");
    GISMapText3->setWordWrap(true);

    auto inputLayout = new QGridLayout(mainWidget);

    auto vspacer3 = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding);
    inputLayout->addWidget(selectComponentsText,0,0,1,3);
    inputLayout->addWidget(GISMapDirectoryLineEdit,1,0);
    inputLayout->addWidget(browseFileButton,1,1);
//    inputLayout->addWidget(loadButton,1,2);
    inputLayout->addWidget(GISMapText1,2,0,1,2);
    inputLayout->addWidget(GISMapText3,3,0,1,2);
    inputLayout->addItem(vspacer3,5,0);


    //    pathToShakeMapDirectory="/Users/steve/Desktop/SimCenter/Examples/ShakeMaps/SanAndreas/";
    //    this->loadShakeMapData();

    return mainWidget;
}



int GISMapWidget::loadGISMapData(void)
{

    // Return if the user cancels
    if(pathToGISMapDirectory.isEmpty() || pathToGISMapDirectory == QDir::currentPath())
    {
        pathToGISMapDirectory = "";
        return -1;
    }


    QFileInfo inputDirInfo(pathToGISMapDirectory);

    if(!inputDirInfo.exists())
    {
        auto relPathToDir = QCoreApplication::applicationDirPath() + QDir::separator() + pathToGISMapDirectory;

        if (!QFileInfo::exists(relPathToDir))
        {
            QString errMsg = "The directory "+ pathToGISMapDirectory+" does not exist check your directory and try again.";
            errorMessage(errMsg);
            return -1;
        }
        else
        {
            pathToGISMapDirectory = relPathToDir;
            GISMapDirectoryLineEdit->setText(pathToGISMapDirectory);
        }

    }


    inputDirInfo = QFileInfo(pathToGISMapDirectory);

    auto inputDir = inputDirInfo.absoluteFilePath();

    QDirIterator iter(inputDir, {"*.geojson", "*.gpkg", "*.tif", "*.shp"}, QDir::Files|QDir::Dirs, QDirIterator::Subdirectories);

    while(iter.hasNext() )
    {
        auto dir = iter.next();

        QFileInfo fileInfo(dir);

        auto fileName = fileInfo.fileName();

        GISMapContainer.insert(fileName,dir);

        listWidget->addItem(fileName);
    }


    auto allItems = listWidget->getListOfItems();

    emit GISMapsListChanged(allItems);

    if(this->getNumGISMapsLoaded() == 0)
    {
        this->errorMessage("Failed to load the GIS Maps. Check the folder and try again.");
        return -1;
    }

    return 0;
}


void GISMapWidget::chooseGISMapDirectoryDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    QString oldPath;

    if(!GISMapDirectoryLineEdit->text().isEmpty())
        oldPath = GISMapDirectoryLineEdit->text();
    else
        oldPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    pathToGISMapDirectory = dialog.getExistingDirectory(this, tr("Folder with GIS files"), oldPath);

    dialog.close();

    if (pathToGISMapDirectory.isEmpty() || pathToGISMapDirectory.isNull())
        this->errorMessage("User did not select a folder");
    else
    {
        pathToGISMapDirectory = pathToGISMapDirectory + QDir::separator();

        // Set file name & entry in line edit
        GISMapDirectoryLineEdit->setText(pathToGISMapDirectory);

        auto res = this->loadGISMapData();

        if(res != 0)
            this->errorMessage("Error, could not load GIS data from the directory "+pathToGISMapDirectory);
    }

    return;
}


bool GISMapWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject nestedObject;

    nestedObject["Directory"] = pathToGISMapDirectory;

    QJsonArray eventsArray;

    for(auto&& it : GISMapContainer)
        eventsArray.append(it);

    nestedObject["Datasets"] = eventsArray;

    jsonObject["GISDatasets"] = nestedObject;

    return true;
}


bool GISMapWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject["Application"] = "UserInputGISMap";

    QJsonObject appData;

    appData["Directory"] = pathToGISMapDirectory;

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool GISMapWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    auto appData = jsonObject.value("ApplicationData").toObject();

    pathToGISMapDirectory = appData.value("Directory").toString();

    GISMapDirectoryLineEdit->setText(pathToGISMapDirectory);

    auto res = this->loadGISMapData();

    if(res != 0)
        return false;
    else
        return true;
}


bool GISMapWidget::inputFromJSON(QJsonObject &jsonObject)
{
    pathToGISMapDirectory = jsonObject.value("Directory").toString();

    if (!pathToGISMapDirectory.isEmpty())
    {
        QFileInfo fileInfo(pathToGISMapDirectory);
        if (fileInfo.exists())
            pathToGISMapDirectory = fileInfo.absoluteFilePath();
        else
        {
            pathToGISMapDirectory = QDir::currentPath() + QDir::separator() + pathToGISMapDirectory;
            QFileInfo fileInfo2(pathToGISMapDirectory);
            if (!fileInfo2.exists())
            {
                this->errorMessage("Path the GIS data directory does not exist");
                return false;
            }
        }

        GISMapDirectoryLineEdit->setText(pathToGISMapDirectory);

        auto res = this->loadGISMapData();

        if(res != 0)
            return false;
        else
            return true;
    }

    return true;
}


bool GISMapWidget::copyFiles(QString &destDir)
{

    Q_UNUSED(destDir);

    return true;
}


int GISMapWidget::getNumGISMapsLoaded()
{
    return listWidget->getNumberOfItems();
}


void GISMapWidget::clear()
{
    listWidget->clear();
    GISMapDirectoryLineEdit->clear();
    pathToGISMapDirectory = "";
    GISMapContainer.clear();
}
