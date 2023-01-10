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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "UserInputFaultWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "SimCenterUnitsWidget.h"

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonObject>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>
#include <QString>

#include "QGISVisualizationWidget.h"

#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>

UserInputFaultWidget::UserInputFaultWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);

    assert(theVisualizationWidget);

    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getUserInputFaultWidget());
    layout->addStretch();
    this->setLayout(layout);


    // Test to remove
//    eventFile = "/Users/steve/Downloads/New\ folder/Task4C_run_Shaking_m5/Input/virtual_fault.csv";
//    this->loadUserGMData();
}


UserInputFaultWidget::~UserInputFaultWidget()
{

}


bool UserInputFaultWidget::outputAppDataToJSON(QJsonObject &jsonObject) {


    jsonObject["Application"] = "UserInputFault";

    QJsonObject appData;

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool UserInputFaultWidget::outputToJSON(QJsonObject &jsonObj)
{

    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        //jsonObj["FaultFile"]=theFile.path();
        jsonObj["FaultFile"]=theFile.absoluteFilePath();
    } else {

        this->errorMessage("Error, the fault rupture file provided "+eventFile+" does not exist, please check the path and try again");
        return false;
    }

    return true;
}


bool UserInputFaultWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    Q_UNUSED(jsonObj);
    return true;
}


bool UserInputFaultWidget::inputFromJSON(QJsonObject &jsonObject)
{
    //QString fileName;
    //QString pathToFile;

    auto thisObject = jsonObject.value("UserDefinedRupture").toObject();

    if (thisObject.contains("FaultFile"))
    {
        eventFile = thisObject["FaultFile"].toString();
        QFileInfo theFile(eventFile);
        if (theFile.exists()) {
            eventFileLineEdit->setText(theFile.absoluteFilePath());
        } else {
            this->errorMessage("Error, the fault rupture file provided "+eventFile+" does not exist, please check the path and try again");
            return false;
        }
    }
    else
    {
        this->infoMessage("Error: The 'FaultFile' key is missing from the json object");
        return false;
    }


    //QString fullFilePath= pathToFile + QDir::separator() + fileName;
    //QString fullFilePath = fileName;
    //eventFile = fileName;


    // load the motions
    this->loadUserGMData();



    return true;
}


QStackedWidget* UserInputFaultWidget::getUserInputFaultWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileInputWidget->setLayout(fileLayout);


    // File main message
    QLabel* mainText = new QLabel(
        "This tab allows you to provide the user-defined CSV file. Please see the example \"Userdef_Rupture_GIS_Input\" under \"Wells and Caprocks\" for a pre-built user-defined CSV file."
    );
    QFont fontMainText = mainText->font();
    fontMainText.setWeight(QFont::Bold);
    mainText->setFont(fontMainText);

    QHBoxLayout* headerTextBox = new QHBoxLayout();
    headerTextBox->addWidget(mainText,0,Qt::AlignLeft);
    headerTextBox->addStretch(1);
    fileLayout->addLayout(headerTextBox,0,0,1,3);


    // getting component file
    QLabel* selectComponentsText = new QLabel("CSV file with user-defined fault ruptures");
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 1,0);
    fileLayout->addWidget(eventFileLineEdit,    1,1);
    fileLayout->addWidget(browseFileButton,     1,2);


    // file columns
    QGroupBox* columnsWidget = new QGroupBox("Note: the user-defined rupture CSV file must contain the following columns");
    QGridLayout* vboxLayout = new QGridLayout(columnsWidget);

    QLabel* col1Label = new QLabel("Column 1 label: EventID");
    QFont fontCol1Label = col1Label->font();
    fontCol1Label.setWeight(QFont::Bold);
    col1Label->setFont(fontCol1Label);
    QLabel* col1Desc = new QLabel("Desc: User-specified IDs (e.g., 1, 2, 3)");

    QLabel* col2Label = new QLabel("Column 2 label: EventName");
    QFont fontCol2Label = col2Label->font();
    fontCol2Label.setWeight(QFont::Bold);
    col2Label->setFont(fontCol2Label);
    QLabel* col2Desc = new QLabel("Desc: User-specified event names");

    QLabel* col3Label = new QLabel("Column 3 label: Magnitude");
    QFont fontCol3Label = col3Label->font();
    fontCol3Label.setWeight(QFont::Bold);
    col3Label->setFont(fontCol3Label);
    QLabel* col3Desc = new QLabel("Desc: Moment magnitude");

    QLabel* col4Label = new QLabel("Column 4 label: AnnualRate");
    QFont fontCol4Label = col4Label->font();
    fontCol4Label.setWeight(QFont::Bold);
    col4Label->setFont(fontCol4Label);
    QLabel* col4Desc = new QLabel("Desc: Mean annual rate");

    QLabel* col5Label = new QLabel("Column 5 label: Dip");
    QFont fontCol5Label = col5Label->font();
    fontCol5Label.setWeight(QFont::Bold);
    col5Label->setFont(fontCol5Label);
    QLabel* col5Desc = new QLabel("Desc: Dip angle (deg)");

    QLabel* col6Label = new QLabel("Column 6 label: Rake");
    QFont fontCol6Label = col6Label->font();
    fontCol6Label.setWeight(QFont::Bold);
    col6Label->setFont(fontCol6Label);
    QLabel* col6Desc = new QLabel("Desc: Rake angle (deg)");

    QLabel* col7Label = new QLabel("Column 7 label: DipDir");
    QFont fontCol7Label = col7Label->font();
    fontCol7Label.setWeight(QFont::Bold);
    col7Label->setFont(fontCol7Label);
    QLabel* col7Desc = new QLabel("Desc: Dip direction (deg)");

    QLabel* col8Label = new QLabel("Column 8 label: UpperDepth");
    QFont fontCol8Label = col8Label->font();
    fontCol8Label.setWeight(QFont::Bold);
    col8Label->setFont(fontCol8Label);
    QLabel* col8Desc = new QLabel("Desc: Upper seismogenic depth (km)");

    QLabel* col9Label = new QLabel("Column 8 label: LowerDepth");
    QFont fontCol9Label = col9Label->font();
    fontCol9Label.setWeight(QFont::Bold);
    col9Label->setFont(fontCol9Label);
    QLabel* col9Desc = new QLabel("Desc: Lower seismogenic depth (km)");

    QLabel* col10Label = new QLabel("Column 8 label: FaultTrace");
    QFont fontCol10Label = col10Label->font();
    fontCol10Label.setWeight(QFont::Bold);
    col10Label->setFont(fontCol10Label);
    QLabel* col10Desc = new QLabel("Desc: List of fault traces (lat/lon,z), with commas (,) as delimiters and brackets ([])to enclose each trace and the entire list (e.g., [[-122,38,0],[-123,38.1,0]]");

    int count = 0;
    vboxLayout->addWidget(col1Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col1Desc,count,1,Qt::AlignLeft);
    count ++;
    \
    vboxLayout->addWidget(col2Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col2Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col3Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col3Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col4Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col4Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col5Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col5Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col6Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col6Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col7Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col7Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col8Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col8Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col9Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col9Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->addWidget(col10Label,count,0,Qt::AlignLeft);
    vboxLayout->addWidget(col10Desc,count,1,Qt::AlignLeft);
    count ++;

    vboxLayout->setColumnStretch(2,1);

    fileLayout->addWidget(columnsWidget,2,0,1,3);


    fileLayout->setRowStretch(3,1);

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user-provdided fault rupture data. This may take a while.");
    progressLabel =  new QLabel(" ");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Select folder containing earthquake fault ruptures");

    return theStackedWidget;
}


void UserInputFaultWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserInputFaultWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserInputFaultWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Fault Rupture File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit
    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);


    QFileInfo eventFileInfo(eventFile);

    if(eventFileInfo.exists())
        this->loadUserGMData();
    else
        this->infoMessage("Warning, the file "+eventFile+" does not exist");

    return;
}




void UserInputFaultWidget::clear(void)
{
    eventFile.clear();
    eventFileLineEdit->clear();

}

void UserInputFaultWidget::loadUserGMData(void)
{

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    this->showProgressBar();

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());
    progressBar->setValue(0);

    // Get the headers
    auto stationDataHeadings = data.front();


    // Create the fields
    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("TabName", QVariant::String));

    for(auto&& it : stationDataHeadings)
        attribFields.push_back(QgsField(it, QVariant::String));

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    int faultIdx = theVisualizationWidget->getIndexOfVal(stationDataHeadings, "FaultTrace");

    if(faultIdx == -1)
    {
        this->infoMessage("Warning, could not find the index for fault trace geometry in "+eventFile+ ". The fault trace heading should be 'FaultTrace' ");
    }

    // The number of headings in the file
    auto numParams = stationDataHeadings.size();


    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        auto faultTrace = rowStr[faultIdx];

        featAttributes[0] = "FaultRuptureTrace";     // "AssetType"
        featAttributes[1] = "Fault Rupture Trace";  // "TabName"

        for(int j = 0; j<numParams; ++j)
        {
            auto str = rowStr[j] ;

            featAttributes[2+j] = str;
        }

        // Create the feature
        QgsFeature feature;

        auto geom = theVisualizationWidget->getMultilineStringGeometryFromJson(faultTrace);
        if(geom.isEmpty())
        {
            this->errorMessage("Error getting the fault trace geometry");
            return;
        }

        feature.setGeometry(geom);


        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }


    auto vectorLayer = theVisualizationWidget->addVectorLayer("linestring", "Fault Ruptures");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        theVisualizationWidget->removeLayer(vectorLayer);
        this->hideProgressBar();
        return;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(2.0);
    selectedLayerMarkerSymbol->setColor(Qt::darkRed);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,vectorLayer);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void UserInputFaultWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    fileInputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}

bool UserInputFaultWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        this->errorMessage("UserInputFaultWidget::copyFiles dest dir does not exist: " + destDir);
        return false;
    }

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
        this->copyFile(eventFile, destDir);
    } else {
        this->errorMessage("UserInputFaultWidget::copyFiles eventFile does not exist: " + eventFile);
        return false;
    }


    return true;
}

void UserInputFaultWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(fileInputWidget);
    progressBarWidget->setVisible(false);
    fileInputWidget->setVisible(true);
}

