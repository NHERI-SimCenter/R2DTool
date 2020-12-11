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
#include "PelicunPostProcessor.h"
#include "VisualizationWidget.h"
#include "SimCenterPreferences.h"
#include "WorkflowAppRDT.h"
#include "GeneralInformationWidget.h"

#include <QPaintEngine>
#include <QGridLayout>
#include <QListWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>
#include <QPrinter>
#include <QDir>

using namespace Esri::ArcGISRuntime;

ResultsWidget::ResultsWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    auto workflowApp = WorkflowAppRDT::getInstance();

    auto analysisName = workflowApp->getGeneralInformationWidget()->getAnalysisName();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5,0,0,0);

    // Header layout and objects
    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(tr("Regional Results Summary"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    // Layout to display the results
//    resultsPageWidget = new QWidget();

    thePelicunPostProcessor = std::make_unique<PelicunPostProcessor>(this,theVisualizationWidget);
    auto pelicanResults = thePelicunPostProcessor->getResultsGridLayout();

    // Export layout and objects
    QHBoxLayout *theExportLayout = new QHBoxLayout();
    QLabel* exportLabel = new QLabel("Export folder:", this);

    exportPathLineEdit = new QLineEdit(this);
    exportPathLineEdit->setMaximumWidth(1000);
    exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    exportPathLineEdit->setText("/Users/steve/Desktop/Results.pdf");

    QPushButton *exportBrowseFileButton = new QPushButton(this);
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportFileButton = new QPushButton(this);
    exportFileButton->setText(tr("Export to PDF"));
    exportFileButton->setMaximumWidth(150);

    connect(exportFileButton,&QPushButton::clicked,this,&ResultsWidget::printToPDF);

    theExportLayout->addStretch();
    theExportLayout->addWidget(exportLabel);
    theExportLayout->addWidget(exportPathLineEdit);
    theExportLayout->addWidget(exportBrowseFileButton);
    theExportLayout->addWidget(exportFileButton);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(pelicanResults);
    mainLayout->addLayout(theExportLayout);

    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&ResultsWidget::assemblePDF);

    this->setMinimumWidth(640);

    this->processResults();
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


int ResultsWidget::processResults()
{

    auto SCPrefs = SimCenterPreferences::getInstance();

    //    auto resultsDirectory = SCPrefs->getLocalWorkDir() + QDir::separator() + "Results";

    QString resultsDirectory = "/Users/steve/Desktop/SimCenter/Examples/rWhaleExample/RDT_example_results_new/";


    if(DVApp.compare("Pelicun") == 0)
    {
        auto res = thePelicunPostProcessor->importResults(resultsDirectory);

        if(res != 0)
        {
            QString err = "Error importing the results from "+resultsDirectory;
            return -1;
        }
    }



    return 0;
}


int ResultsWidget::printToPDF(void)
{
    theVisualizationWidget->takeScreenShot();

    return 0;
}


int ResultsWidget::assemblePDF(QImage screenShot)
{

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageMargins(12, 16, 12, 20, QPrinter::Millimeter);

    auto outputFileName = exportPathLineEdit->text();

    if(outputFileName.isEmpty())
    {
        QString errMsg = "The file name is empty";
        this->userMessageDialog(errMsg);
        return -1;
    }

    printer.setOutputFileName(outputFileName);


    QPainter painter;
    painter.begin (&printer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto scale = 10.0;
    painter.drawImage (QRect (0,0,scale*screenShot.width(), scale*screenShot.height()), screenShot);

    painter.end ();

    return 0;
}
