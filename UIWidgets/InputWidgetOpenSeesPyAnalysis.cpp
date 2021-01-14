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

// Written by: Frank McKenna, Stevan Gavrilovic

#include "InputWidgetOpenSeesPyAnalysis.h"
#include "RandomVariablesContainer.h"

#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>

InputWidgetOpenSeesPyAnalysis::InputWidgetOpenSeesPyAnalysis(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(theRandomVariableIW)
{   
    //
    // create layout for all qLineEdits
    //

    QGridLayout *layout = new QGridLayout();


    //
    // for each QlineEdit create a label, the Qline Edit and set some defaults
    //

    int row = 0;

//    QLabel *labelAnalysis = new QLabel("Analysis: ");
//    layout->addWidget(labelAnalysis, row, 0);
//    theAnalysis = new QLineEdit("Transient -numSubLevels 2 -numSubSteps 10");
//    theAnalysis->setToolTip(tr("Command for creating the analysis"));
//    layout->addWidget(theAnalysis, row, 1);
//    row++;

//    QLabel *label2 = new QLabel("Integration: ");
//    layout->addWidget(label2, row, 0);
//    theIntegration = new QLineEdit();
//    theIntegration->setText("Newmark 0.5 0.25");
//    theIntegration->setToolTip(tr("Command specifying integration scheme"));
//    layout->addWidget(theIntegration, row, 1);
//    row++;

//    QLabel *label1 = new QLabel("Algorithm: ");
//    layout->addWidget(label1, row, 0);
//    //theAlgorithm = new QLineEdit("Newton");
//    theAlgorithm = new QComboBox();
//    theAlgorithm->setToolTip(tr("Nonlinear Solution Algorithm"));
//    theAlgorithm->addItem("Linear");
//    theAlgorithm->addItem("Newton");
//    theAlgorithm->addItem("Newmark");
//    theAlgorithm->setEditable(false);
//    theAlgorithm->setCurrentText("Newton");
//    layout->addWidget(theAlgorithm, row, 1);
//    row++;

//    QLabel *label3 = new QLabel();
//    label3->setText(QString("ConvergenceTest: "));
//    layout->addWidget(label3, row, 0);
//    theConvergenceTest = new QLineEdit();
//    theConvergenceTest->setText("NormUnbalance 1.0e-2 10");
//    theConvergenceTest->setToolTip(tr("Convergence test command used in script: type tolerance and # iterations, valid types are NormUnbalance, NormDispIncr, NormEnergy are options"));
//    layout->addWidget(theConvergenceTest, row, 1);
//    row++;

//    QLabel *labelSolver = new QLabel("Solver: ");
//    layout->addWidget(labelSolver, row, 0);
//    theSolver = new QLineEdit("Umfpack");
//    theSolver->setToolTip(tr("Equation Solver, Umfpack, ProfileSPD, BandGeneral, BandSPD, FullGeneral"));
//    layout->addWidget(theSolver, row, 1);
//    row++;


//    //
//    // Damping Options
//    //    Modal or Rayleigh

//    QLabel *labelDampingSelection = new QLabel("Damping Model");

//    theSelectionBox = new QComboBox();
//    theSelectionBox->addItem(tr("Rayleigh Damping"));
//    theSelectionBox->addItem(tr("Modal Damping"));
//    theStackedWidget = new QStackedWidget();

//    layout->addWidget(labelDampingSelection, row, 0);
//    layout->addWidget(theSelectionBox, row, 1);
//    row++;

//    layout->addWidget(theStackedWidget, row, 0, 1, 2);
//    row++;

//    //
//    // rayleigh option
//    //

//    QFrame *theRayleighWidget = new QFrame();
//    theRayleighWidget->setFrameStyle(QFrame::Panel | QFrame::Sunken);
//    //theRayleighWidget->setLineWidth(1);
//    QGridLayout *layoutRayleigh = new QGridLayout();

//    QLabel *labelRD = new QLabel();
//    labelRD->setText(QString("Damping Ratio: "));
//    layoutRayleigh->addWidget(labelRD, 0, 0);
//    dampingRatio = new QLineEdit();
//    dampingRatio->setText("0.02");
//    dampingRatio->setToolTip(tr("Damp ratio, 0.02 = 2% damping"));
//    layoutRayleigh->addWidget(dampingRatio, 0, 1);

//    QLabel *labelRM1 = new QLabel();
//    labelRM1->setText(QString("Mode 1: "));
//    layoutRayleigh->addWidget(labelRM1, 2, 0);
//    QLabel *labelRM2 = new QLabel();
//    labelRM2->setText(QString("Mode 2: "));
//    layoutRayleigh->addWidget(labelRM2, 3, 0);
//    theRayleighStiffness = new QComboBox();

//    QLabel *labelStiffness = new QLabel(QString("Selected Tangent Stiffness: "));
//    theRayleighStiffness->addItem(tr("Initial"));
//    theRayleighStiffness->addItem(tr("Current"));
//    theRayleighStiffness->addItem(tr("Committed"));
//    layoutRayleigh->addWidget(labelStiffness, 1,0);
//    layoutRayleigh->addWidget(theRayleighStiffness, 1,1);

//    firstMode = new QLineEdit();
//    firstMode->setText("1");
//    firstMode->setToolTip(tr("First Mode to to determine damping ratio, if 0 stiffness proportional damping"));
//    secondMode = new QLineEdit();
//    secondMode->setText("0");
//    secondMode->setToolTip(tr("Second Mode to to determine damping ratio, if 0 mass proportional damping"));
//    firstMode->setValidator(new QIntValidator);
//    secondMode->setValidator(new QIntValidator);

//    layoutRayleigh->addWidget(firstMode, 2, 1);
//    layoutRayleigh->addWidget(secondMode, 3, 1);
//    theRayleighWidget->setLayout(layoutRayleigh);

//    theStackedWidget->addWidget(theRayleighWidget);

//    //
//    // modal option
//    //

//    QFrame *theModalWidget = new QFrame();
//    theModalWidget->setFrameStyle(QFrame::Panel | QFrame::Sunken);
//    //theModalWidget->setLineWidth(1);
//    QGridLayout *layoutModal = new QGridLayout();

//    QLabel *labelMDR = new QLabel();
//    labelMDR->setText(QString("Damping Ratio: "));
//    layoutModal->addWidget(labelMDR, 0, 0);
//    dampingRatioModal = new QLineEdit();
//    dampingRatioModal->setText("0.02");
//    dampingRatioModal->setToolTip(tr("Damp ratio, 0.02 = 2% damping"));
//    layoutModal->addWidget(dampingRatioModal, 0, 1);

//    QLabel *labelNumModes = new QLabel();
//    labelNumModes->setText(QString("# Modes: "));
//    layoutModal->addWidget(labelNumModes, 1, 0);
//    numModesModal = new QLineEdit();
//    numModesModal->setText("1");
//    numModesModal->setToolTip(tr("number of modes to include"));
//    numModesModal->setValidator(new QIntValidator);

//    QLabel *labelMDRT = new QLabel(QString("Stiffness Proportional Damping Ratio: "));
//    layoutModal->addWidget(labelMDRT, 2, 0);
//    dampingRatioModalTangent = new QLineEdit();
//    dampingRatioModalTangent->setText("0.0");
//    dampingRatioModalTangent->setToolTip(tr("Stiffness Proportional Damping ratio at last mode, 0.02 = 2% damping"));
//    dampingRatioModalTangent->setValidator(new QDoubleValidator);
//    layoutModal->addWidget(dampingRatioModalTangent, 2, 1);

//    layoutModal->addWidget(numModesModal, 1, 1);
//    theModalWidget->setLayout(layoutModal);

//    theStackedWidget->addWidget(theModalWidget);

//    QLabel *labelFile = new QLabel();
//    labelFile->setText("Analysis Script: ");
//    file = new QLineEdit;
//    file->setToolTip(tr("User provided analysis script, replaces OpenSees default"));
//    layout->addWidget(labelFile, row, 0);
//    layout->addWidget(file, row, 1);

//    QPushButton *chooseFile = new QPushButton();
//    chooseFile->setText(tr("Choose"));
//    layout->addWidget(chooseFile, row, 2);
//    row++;

//    connect(dampingRatio,SIGNAL(editingFinished()), this, SLOT(dampingEditingFinished()));
//    //connect(theTolerance,SIGNAL(editingFinished()), this, SLOT(toleranceEditingFinished()));
//    connect(chooseFile, SIGNAL(clicked(bool)), this, SLOT(chooseFileName()));
//    connect(theSelectionBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changedDampingMethod(QString)));

    QWidget *dummy = new QWidget();
    layout->addWidget(dummy,8,0);
    layout->setRowStretch(row,1);
    layout->setColumnStretch(3,2);
    layout->setColumnStretch(1,1);

    // set the widgets layout
    this->setLayout(layout);


   //this->setMinimumWidth(200);
   // this->setMaximumWidth(400);
}

InputWidgetOpenSeesPyAnalysis::~InputWidgetOpenSeesPyAnalysis() {

}


void InputWidgetOpenSeesPyAnalysis::changedDampingMethod(QString newMethod) {
    if (newMethod == "Rayleigh Damping")
        theStackedWidget->setCurrentIndex(0);
    else
        theStackedWidget->setCurrentIndex(1);

}


void InputWidgetOpenSeesPyAnalysis::clear(void)
{
    theAnalysis->setText("Transient -numSubLevels 2 -numSubSteps 10");
    theIntegration->setText("Newmark 0.5 0.25");
    theSolver->setText("Umfpack");
    //theAlgorithm->setText("Newmark");
    theAlgorithm->setCurrentText("Newmark");
    theConvergenceTest->setText("NormUnbalance 1.0e-2 10");

    dampingRatio->setText("0.02");
    firstMode->setText("1");
    secondMode->setText("0");
    dampingRatioModal->setText("0.02");
    numModesModal->setText("1");
    theSelectionBox->setCurrentIndex(0);
    dampingRatioModalTangent->setText("0.0");
    theRayleighStiffness->setCurrentIndex(0);

    file->setText("");
}


bool InputWidgetOpenSeesPyAnalysis::outputToJSON(QJsonObject &jsonObject)
{
    bool result = true;
    jsonObject["Application"] = "OpenSeesPy-Simulation";

    QJsonObject dataObj;
    dataObj["analysis"]=theAnalysis->text();
    dataObj["integration"]=theIntegration->text();
    dataObj["algorithm"]=theAlgorithm->currentText();
    dataObj["solver"]=theSolver->text();
    dataObj["convergenceTest"]=theConvergenceTest->text();
    dataObj["dampingModel"]= theSelectionBox->currentText();
    dataObj["firstMode"]=firstMode->text().QString::toInt();
    dataObj["secondMode"]=secondMode->text().QString::toInt();
    dataObj["numModesModal"]=numModesModal->text().QString::toInt();
    dataObj["rayleighTangent"]=theRayleighStiffness->currentText();
    dataObj["modalRayleighTangentRatio"]=dampingRatioModalTangent->text().QString::toDouble();

    bool ok;

    QString dampText = dampingRatio->text();
    double dampDouble = dampText.QString::toDouble(&ok);
    if (ok == true)
        dataObj["dampingRatio"]=dampDouble;
    else
        dataObj["dampingRatio"]= QString("RV.") + dampText;

    QString dampModalText = dampingRatioModal->text();
    double dampModalDouble = dampModalText.QString::toDouble(&ok);
    if (ok == true)
        dataObj["dampingRatioModal"]=dampModalDouble;
    else
        dataObj["dampingRatioModal"]= QString("RV.") + dampModalText;


    if (!file->text().isEmpty() && !file->text().isNull()) {
        QFileInfo fileInfo(file->text());
        dataObj["fileName"]= fileInfo.fileName();
        dataObj["filePath"]=fileInfo.path();
    }

    jsonObject["ApplicationData"] = QJsonObject();//dataObj;


    return result;
}


bool InputWidgetOpenSeesPyAnalysis::inputFromJSON(QJsonObject &jsonObject)
{
    bool result = true;
    this->clear();

    if (jsonObject.contains("integration") && jsonObject.contains("algorithm")
	&& jsonObject.contains("convergenceTest")) {

        // theAlgorithm->setText(jsonObject["algorithm"].toString());
        theAlgorithm->setCurrentText(jsonObject["algorithm"].toString());
        theConvergenceTest->setText(jsonObject["convergenceTest"].toString());
        theIntegration->setText(jsonObject["integration"].toString());

    } else {
        emit sendErrorMessage("ERROR: InputWidgetOpenSeesPyAnalysis - no \"integration\" ,\"convergenceTest\" or \"algorithm\" data");
        return false;
    }


    if (jsonObject.contains("dampingRatio")) {
        QJsonValue theValue = jsonObject["dampingRatio"];
        if (theValue.isString()) {
            QString text = theValue.toString();
            text.remove(0,3); // remove RV.
            dampingRatio->setText(text);
       } else if (theValue.isDouble())
            dampingRatio->setText(QString::number(theValue.toDouble()));
    } else {
      emit sendErrorMessage("ERROR: InputWidgetOpenSeesPyAnalysis - no \"dampingRatio\" data");
        return false;
    }

    if (jsonObject.contains("dampingRatioModal")) {
        QJsonValue theValue = jsonObject["dampingRatioModal"];
        if (theValue.isString()) {
            QString text = theValue.toString();
            text.remove(0,3); // remove RV.
            dampingRatioModal->setText(text);
       } else if (theValue.isDouble())
            dampingRatioModal->setText(QString::number(theValue.toDouble()));
    } else {
           dampingRatioModal->setText("0.02");// old code, use defaults
    }

    if (jsonObject.contains("dampingModel")) {
        QJsonValue theValue = jsonObject["dampingModel"];
        if (theValue.isString())
            theSelectionBox->setCurrentText(theValue.toString());
    } else {

        // old file .. use defaults


        numModesModal->setText("1");
        theSelectionBox->setCurrentIndex(0);
        theConvergenceTest->setText("NormUnbalance 1.0e-2 10");
        theSolver->setText("Umfpack");
        theAnalysis->setText("Transient -numSubLevels 2 -numSubSteps 10");
    }

    if (jsonObject.contains("firstMode")) {
        QJsonValue theValue = jsonObject["firstMode"];
         if (theValue.isDouble())
            firstMode->setText(QString::number(theValue.toInt()));
    } else {
        firstMode->setText("1"); // old code default
    }

    if (jsonObject.contains("secondMode")) {
        QJsonValue theValue = jsonObject["secondMode"];
        if (theValue.isDouble())
            secondMode->setText(QString::number(theValue.toInt()));
    } else {
        secondMode->setText("0"); // old code
    }

    if (jsonObject.contains("numModesModal")) {
        QJsonValue theValue = jsonObject["numModesModal"];
        if (theValue.isDouble())
            numModesModal->setText(QString::number(theValue.toInt()));
    } else {
        ; // old code
    }

    if (jsonObject.contains("rayleighTangent")) {
        QJsonValue theValue = jsonObject["rayleighTangent"];
        if (theValue.isString())
            theRayleighStiffness->setCurrentText(theValue.toString());
    } else {
        ; // old code
    }

    if (jsonObject.contains("modalRayleighTangentRatio")) {
        QJsonValue theValue = jsonObject["modalRayleighTangentRatio"];
        if (theValue.isDouble())
            dampingRatioModalTangent->setText(QString::number(theValue.toDouble()));
    } else {
        ; // old code
    }

    if (jsonObject.contains("analysis")) {
        QJsonValue theValue = jsonObject["analysis"];
        theAnalysis->setText(theValue.toString());
    } else {
        theAnalysis->setText("Transient -numSubLevels 2 -numSubSteps 10");
        ; // old code
    }

    if (jsonObject.contains("solver")) {
        QJsonValue theValue = jsonObject["solver"];
        theSolver->setText(theValue.toString());
    } else {
        theSolver->setText("Umfpack");
        ; // old code
    }

    if (jsonObject.contains("fileName")) {
        QJsonValue theName = jsonObject["fileName"];
        QString fileName = theName.toString();
        if (jsonObject.contains("filePath")) {
            QJsonValue theName = jsonObject["filePath"];
            QString filePath = theName.toString();
            file->setText(QDir(filePath).filePath(fileName));
        } else
            return false;
    }


    return result;
}


bool InputWidgetOpenSeesPyAnalysis::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"] = "OpenSeesPy-Simulation";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;

    return true;
}


bool InputWidgetOpenSeesPyAnalysis::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return true;
}


void InputWidgetOpenSeesPyAnalysis::chooseFileName(void) {
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "All files (*.*)");
    file->setText(fileName);
}


bool InputWidgetOpenSeesPyAnalysis::copyFiles(QString &dirName) {
    if (file->text().isNull() || file->text().isEmpty()) {
        return true;
    }
    if  (this->copyFile(file->text(), dirName) ==  false) {
        emit sendErrorMessage(QString("ERROR: OpenSees Analysis copyFiles: failed to copy file: ") +file->text());
        return false;
    }
    return true;
}


// need to check if a random variable
void InputWidgetOpenSeesPyAnalysis::dampingEditingFinished() {
    QString text = dampingRatio->text();
    bool ok;
    double dampDouble = text.QString::toDouble(&ok);
    Q_UNUSED(dampDouble);

    if (ok == false) {
        qDebug() << text << " " << lastDampingRatio;

        if (text != lastDampingRatio) {
            QStringList rvs;
            rvs.append(text);
            rvs.append("0.02");
            theRandomVariablesContainer->addConstantRVs(rvs);
            lastDampingRatio = text;
        }
    }
}


void InputWidgetOpenSeesPyAnalysis::toleranceEditingFinished() {
    QString text = "BLAH"; // theTolerance->text();
    bool ok;
    double tolDouble = text.QString::toDouble(&ok);
    Q_UNUSED(tolDouble);

    if (ok == false) {
        if (text != lastTolerance) {
            QStringList rvs;
            rvs.append(text);
            rvs.append("0.02");
            theRandomVariablesContainer->addConstantRVs(rvs);
            lastTolerance = text;
        }
    }
}
