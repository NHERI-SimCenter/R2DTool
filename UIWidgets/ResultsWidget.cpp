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

#include "AssetInputDelegate.h"
#include "GeneralInformationWidget.h"
#include "PelicunPostProcessor.h"
#include "ResultsWidget.h"
#include "SimCenterPreferences.h"
#include "VisualizationWidget.h"
#include <WorkflowAppR2D.h>
#include "sectiontitle.h"

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPushButton>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFileDialog>

using namespace Esri::ArcGISRuntime;

ResultsWidget::ResultsWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    DVApp = "Pelicun";

    resultsMainLabel = new QLabel("No results to display", this);

    mainStackedWidget = new QStackedWidget(this);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5,0,0,0);

    // Header layout and objects
    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(tr("Regional Results Summary"));
    label->setMinimumWidth(150);

    auto disclaimerLabel = new QLabel("Disclaimer: The presented simulation results are not representative of any individual building’s response. To understand the response of any individual building, "
                                     "please consult with a professional structural engineer. The presented tool does not assert the known condition of the building. Just as it cannot be used to predict the negative outcome of an individual "
                                     "building, prediction of safety or an undamaged state is not assured for an individual building. Any opinions, findings, and conclusions or recommendations expressed in this material are "
                                     "those of the author(s) and do not necessarily reflect the views of the National Science Foundation.",this);
    disclaimerLabel->setStyleSheet("font: 10pt;");
    disclaimerLabel->setWordWrap(true);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(10,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addWidget(disclaimerLabel,Qt::AlignLeft);

    //    theHeaderLayout->addStretch(1);

    // Layout to display the results
    resultsPageWidget = new QWidget();

    mainStackedWidget->addWidget(resultsPageWidget);

    QVBoxLayout* resultsPlaceHolderLayout = new QVBoxLayout(resultsPageWidget);
    resultsPlaceHolderLayout->addStretch();
    resultsPlaceHolderLayout->addWidget(resultsMainLabel,0,Qt::AlignCenter);
    resultsPlaceHolderLayout->addStretch();

    thePelicunPostProcessor = std::make_unique<PelicunPostProcessor>(parent,theVisualizationWidget);

    mainStackedWidget->addWidget(thePelicunPostProcessor.get());

    // Export layout and objects
    QGridLayout *theExportLayout = new QGridLayout();

    exportLabel = new QLabel("Export folder:", this);
    exportPathLineEdit = new QLineEdit(this);
    //exportPathLineEdit->setMaximumWidth(1000);
    // exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QString defaultOutput = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + QString("Results.pdf");
    exportPathLineEdit->setText(defaultOutput);

    exportBrowseFileButton = new QPushButton(this);
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    connect(exportBrowseFileButton,&QPushButton::clicked,this,&ResultsWidget::chooseResultsDirDialog);

    exportPDFFileButton = new QPushButton(this);
    exportPDFFileButton->setText(tr("Export to PDF"));

    connect(exportPDFFileButton,&QPushButton::clicked,this,&ResultsWidget::printToPDF);

    selectComponentsText = new QLabel("Select a subset of buildings to display the results:",this);
    selectComponentsLineEdit = new AssetInputDelegate();

    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&ResultsWidget::handleComponentSelection);

    selectComponentsButton = new QPushButton();
    selectComponentsButton->setText(tr("Select"));
    selectComponentsButton->setMaximumWidth(150);

    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));

    // theExportLayout->addStretch();
    theExportLayout->addWidget(selectComponentsText,     0,0);
    theExportLayout->addWidget(selectComponentsLineEdit, 0,1);
    theExportLayout->addWidget(selectComponentsButton,   0,2);
    // theExportLayout->addStretch();
    theExportLayout->addWidget(exportLabel,            1,0);
    theExportLayout->addWidget(exportPathLineEdit,     1,1);
    theExportLayout->addWidget(exportBrowseFileButton, 1,2);
    theExportLayout->addWidget(exportPDFFileButton,       2,0,1,3);

    // theExportLayout->addStretch();
    theExportLayout->setRowStretch(3,1);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(mainStackedWidget);
    mainLayout->addLayout(theExportLayout,1);
    mainLayout->addStretch(1);

    this->resultsShow(false);

    this->setMinimumWidth(640);
}


ResultsWidget::~ResultsWidget()
{

}


void ResultsWidget::resultsShow(bool value)
{
    if(!value)
    {
        thePelicunPostProcessor->setIsVisible(false);
        mainStackedWidget->setCurrentWidget(resultsPageWidget);
        selectComponentsButton->hide();
        selectComponentsLineEdit->hide();
        selectComponentsText->hide();
        exportPDFFileButton->hide();
        exportBrowseFileButton->hide();
        exportPathLineEdit->hide();
        exportLabel->hide();
    }
    else
    {
        thePelicunPostProcessor->setIsVisible(true);
        mainStackedWidget->setCurrentWidget(thePelicunPostProcessor.get());
        selectComponentsButton->show();
        selectComponentsLineEdit->show();
        selectComponentsText->show();
        exportPDFFileButton->show();
        exportBrowseFileButton->show();
        exportPathLineEdit->show();
        exportLabel->show();
    }
}


bool ResultsWidget::outputToJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


bool ResultsWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


void ResultsWidget::setCurrentlyViewable(bool status){

    if (status == true)
        thePelicunPostProcessor->setCurrentlyViewable(status);
}


int ResultsWidget::processResults(QString resultsDirectory)
{

    //auto SCPrefs = SimCenterPreferences::getInstance();

    //auto resultsDirectory = SCPrefs->getLocalWorkDir() + QDir::separator() + "tmp.SimCenter" + QDir::separator() + "Results";

    qDebug() << resultsDirectory;

    try
    {
        if(DVApp.compare("Pelicun") == 0)
        {
            thePelicunPostProcessor->importResults(resultsDirectory);

            this->resultsShow(true);
        }
    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);

        return -1;
    }


    return 0;
}


int ResultsWidget::printToPDF(void)
{
    auto outputFileName = exportPathLineEdit->text();

    if(outputFileName.isEmpty())
    {
        QString errMsg = "The file name is empty";
        this->userMessageDialog(errMsg);
        return -1;
    }

    QDir theDir(outputFileName);
    if (theDir.exists()) {
        QMessageBox msgBox;
        msgBox.setText("Output file is a directory. No file will be written.");
        msgBox.exec();
        return 0;
    }

    QFile theFile(outputFileName);
    if (theFile.exists()) {
     QMessageBox::StandardButton reply = QMessageBox::question(this,
                                      "File Exists", "File Exists .. Do you want to overwrite?",
                             QMessageBox::Yes | QMessageBox::No);
       if(reply == QMessageBox::No) {
           return 0;
       }
    }


    if(DVApp.compare("Pelicun") == 0) {

        int res = thePelicunPostProcessor->printToPDF(outputFileName);

        if(res != 0) {
            QString err = "Error printing the PDF";
            this->userMessageDialog(err);
            return -1;
        }
    }

    return 0;
}


void ResultsWidget::selectComponents(void)
{
    selectComponentsLineEdit->clear();

    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);
    }
}


void ResultsWidget::handleComponentSelection(void)
{

    try
    {
        if(DVApp.compare("Pelicun") == 0)
        {
            auto IDSet = selectComponentsLineEdit->getSelectedComponentIDs();
            thePelicunPostProcessor->processResultsSubset(IDSet);
        }

    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);
    }
}


void ResultsWidget::chooseResultsDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Directory to Save Results PDF"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty())
    {
        return;
    }

    newPath += QDir::separator() + QString("Results.pdf");

    exportPathLineEdit->setText(newPath);

    return;
}


void ResultsWidget::clear(void)
{
    QString defaultOutput = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + QString("Results.pdf");
    exportPathLineEdit->setText(defaultOutput);
    selectComponentsLineEdit->clear();

    thePelicunPostProcessor->clear();

    resultsShow(false);
}


