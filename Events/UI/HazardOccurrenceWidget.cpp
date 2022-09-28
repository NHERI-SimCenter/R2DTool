/* *****************************************************************************
Copyright (c) 2016-2022, The Regents of the University of California (Regents).
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

// Written by: Kuanshi Zhong

#include "HazardOccurrenceWidget.h"
#include "HazardOccurrence.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QSpacerItem>
#include <HBoxFormLayout.h>
#include <QFileInfo>
#include <QDoubleValidator>
#include <sstream>
#include <QFileDialog>

HazardOccurrenceWidget::HazardOccurrenceWidget(QWidget *parent) : QWidget(parent)
{
    //We use a grid layout for the widget
    QGridLayout* layout = new QGridLayout(this);

    QLabel* candidEQlabel = new QLabel(tr("Candidiate earthquake number:"),this);
    QLabel* modelLabel = new QLabel(tr("Forecast model:"),this);

    ModelTypeCombo = new QComboBox(this);
    ModelTypeCombo->addItem("WGCEP (2007) UCERF2 - Single Branch");
    ModelTypeCombo->addItem("Mean UCERF3");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.1");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.2");
    ModelTypeCombo->setToolTip("Select where are the candidate earthquakes from");

    candidEQLineEdit = new QLineEdit(this);
    candidEQLineEdit->setText("100");
    candidEQLineEdit->setToolTip("How many candidate earthquakes: fill in an integer number or \'All\' for including all possible ruptures");

    // number of scenarios
    QLabel* NumScenarioLabel= new QLabel(tr("Reduced earthquake number:"),this);
    NumScenarioLineEdit = new QLineEdit(this);
    NumScenarioLineEdit->setText("10");
    auto validator = new QIntValidator(1, 9999, this);
    NumScenarioLineEdit->setValidator(validator);
    NumScenarioLineEdit->setToolTip("The maximum earthquake number in the reduced set: an integer number");

    // number of ground motion maps
    QLabel* NumGMMapLabel= new QLabel(tr("Reduced motion number:"),this);
    NumGMMapLineEdit = new QLineEdit(this);
    NumGMMapLineEdit->setText("100");
    auto validator_gmm = new QIntValidator(1, 9999, this);
    NumGMMapLineEdit->setValidator(validator_gmm);
    NumGMMapLineEdit->setToolTip("The maximum ground motion number in the reduced set: an integer number");

    // Hazard occurrence model
    QLabel* HOModelLabel = new QLabel(tr("Hazard occurrence model:"),this);
    HOModelTypeCombo = new QComboBox(this);
    HOModelTypeCombo->addItem("Manzour & Davidson (2016)");

    // Hazard curve box
    QLabel* HazardCurveType= new QLabel(tr("Hazard curve input:"),this);
    HCTypeCombo = new QComboBox(this);
    HCTypeCombo->addItem("Inferred");
    HCTypeCombo->addItem("User-defined");
    HCTypeCombo->setToolTip("\'Inferred\' will query USGS hazard curve if available");
    QLabel* filenameLabel = new QLabel(tr("Hazard curve file:"),this);
    FilenameLineEdit = new QLineEdit(this);
    FilenameLineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    FilenameLineEdit->setDisabled(1);
    FilenameLineEdit->setStyleSheet("background-color: lightgray");
    FilenameLineEdit->setToolTip("The csv and json formats are supported - please check the online documentation for templates");
    browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setDisabled(1);
    browseFileButton->setMaximumWidth(150);

    // NSHM edition
    QLabel* nshm_ed_label= new QLabel(tr("National seimsic hazard map:"),this);
    NSHM_Edition_Combo = new QComboBox(this);
    NSHM_Edition_Combo->addItem("E2014");
    NSHM_Edition_Combo->addItem("E2008");
    NSHM_Edition_Combo->setToolTip("Select national seismic hazard map version for inferred hazard curves");

    // Intensity measure type
    QLabel* imt_label= new QLabel(tr("Intensity measure:"),this);
    IMT_Combo = new QComboBox(this);
    IMT_Combo->addItem("PGA");
    IMT_Combo->addItem("SA");
    IMT_Combo->setToolTip("Select intensity measure type for the hazard curves");

    // Sa period
    IMT_period= new QLabel(tr("Hazard curve Sa period:"),this);
    PeriodEdit = new QLineEdit(this);
    PeriodEdit->setText("0.0");
    PeriodEdit->setDisabled(1);
    PeriodEdit->setStyleSheet("background-color: lightgray");
    PeriodEdit->setToolTip("Fill in a float number for the period if 'SA' is the intensity measure type");

    QLabel* return_period_label = new QLabel(tr("Return periods (yr):"),this);

    // Set a validator to allow only numbers, periods, and spaces
    QRegExp regExpAllow("^([1-9][0-9]*|[1-9]*\\.[0-9]*|0\\.[0-9]*)*(([ ]*,[ ]*){0,1}([[1-9]*\\.[0-9]*|[1-9][0-9]*|0\\.[0-9]*))*");
    LEValidator = new QRegExpValidator(regExpAllow,this);
    return_periods_lineEdit = new QLineEdit(this);
    return_periods_lineEdit->setText("224, 475, 975, 2475");
    return_periods_lineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    return_periods_lineEdit->setToolTip("Fill in an integer list separated by commas for return periods in year");

    // Add a horizontal
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(HOModelLabel,0,0);
    layout->addWidget(HOModelTypeCombo,0,1,1,2);
    layout->addWidget(modelLabel,0,3);
    layout->addWidget(ModelTypeCombo,0,4,1,3);

    layout->addWidget(candidEQlabel,2,0);
    layout->addWidget(candidEQLineEdit,2,1,1,1);
    layout->addWidget(NumScenarioLabel,2,2);
    layout->addWidget(NumScenarioLineEdit,2,3,1,1);
    layout->addWidget(NumGMMapLabel,2,4);
    layout->addWidget(NumGMMapLineEdit,2,5,1,2);

    layout->addWidget(HazardCurveType,3,0);
    layout->addWidget(HCTypeCombo,3,1,1,1);
    layout->addWidget(nshm_ed_label,3,2,1,1);
    layout->addWidget(NSHM_Edition_Combo,3,3,1,1);
    layout->addWidget(filenameLabel,3,4,1,1);
    layout->addWidget(FilenameLineEdit,3,5,1,1);
    layout->addWidget(browseFileButton,3,6,1,1);

    layout->addWidget(imt_label,4,0);
    layout->addWidget(IMT_Combo,4,1,1,1);
    layout->addWidget(IMT_period,4,2,1,1);
    layout->addWidget(PeriodEdit,4,3,1,1);
    layout->addWidget(return_period_label,4,4);
    layout->addWidget(return_periods_lineEdit,4,5,1,2);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new HazardOccurrence(ModelTypeCombo->currentText(),"", \
                                       HOModelTypeCombo->currentText(), \
                                       candidEQLineEdit->text(), \
                                       NumScenarioLineEdit->text().toInt(), \
                                       NumGMMapLineEdit->text().toInt(),
                                       FilenameLineEdit->text(), \
                                       NSHM_Edition_Combo->currentText(), \
                                       IMT_Combo->currentText(), \
                                       PeriodEdit->text().toDouble(), this);

    this->setHCFile("");
    this->candidEQLineEdit->setText(m_eqRupture->getCandidateEQ());
    this->NumScenarioLineEdit->setText(QString::number(m_eqRupture->getNumScen()));

    this->setupConnections();
}



HazardOccurrence* HazardOccurrenceWidget::getRuptureSource() const
{
    return m_eqRupture;
}


void HazardOccurrenceWidget::setupConnections()
{
    connect(this->candidEQLineEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &HazardOccurrence::setCandidateEQ);

    connect(this->NumScenarioLineEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &HazardOccurrence::setNumScen);

    connect(this->NumGMMapLineEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &HazardOccurrence::setNumGMMap);

    connect(this->ModelTypeCombo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &HazardOccurrence::setEQModelType);

    connect(this->HOModelTypeCombo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &HazardOccurrence::setHazardOccurrenceModel);

    connect(this->HCTypeCombo, &QComboBox::currentTextChanged,
            this, &HazardOccurrenceWidget::setHCType);

    connect(this->NSHM_Edition_Combo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &HazardOccurrence::setNSHMEdition);

    connect(this->IMT_Combo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &HazardOccurrence::setIMType);

    connect(this->IMT_Combo, &QComboBox::currentTextChanged,
            this, &HazardOccurrenceWidget::handleTypeChanged);

    connect(this->PeriodEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &HazardOccurrence::setPeriod);

    connect(this->return_periods_lineEdit, &QLineEdit::editingFinished,
            this, &HazardOccurrenceWidget::commitReturnPeriods);

    connect(this->FilenameLineEdit, &QLineEdit::textChanged,
            this, &HazardOccurrenceWidget::setHCFile);

    connect(browseFileButton, &QPushButton::clicked,
            this, &HazardOccurrenceWidget::loadHazardCurveFile);
}


void HazardOccurrenceWidget::setHCType(const QString value)
{
    if (value.compare("Inferred")==0) {
        // inferred
        this->m_eqRupture->setHazardCurve(value);
        this->FilenameLineEdit->setDisabled(1);
        this->browseFileButton->setDisabled(1);
        FilenameLineEdit->setStyleSheet("background-color: lightgray");
        this->NSHM_Edition_Combo->setEnabled(1);
    } else {
        this->FilenameLineEdit->setEnabled(1);
        this->browseFileButton->setEnabled(1);
        FilenameLineEdit->setStyleSheet("background-color: white");
        this->NSHM_Edition_Combo->setDisabled(1);
    }
}


void HazardOccurrenceWidget::setHCFile(const QString value)
{
    if (this->HCTypeCombo->currentText().compare("User-defined")==0) {
        this->FilenameLineEdit->setText(value);
        QFileInfo fi(value);
        QString name = fi.fileName();
        this->m_eqRupture->setHazardCurve(name);
    } else {
        this->m_eqRupture->setHazardCurve(this->HCTypeCombo->currentText());
    }
}


void HazardOccurrenceWidget::setIMType(const QString value)
{
    this->m_eqRupture->setIMType(value);
}


QString HazardOccurrenceWidget::checkReturnPeriodsValid(const QString& input) const
{

    QString validInput = input;

    if(validInput.isEmpty())
        return validInput;

    int pos = 0;
    if(LEValidator->validate(const_cast<QString&>(input), pos) != 1)
    {
        validInput = QStringRef(&input, 0, pos-1).toString();

        qDebug()<<"pos"<<pos<<" : "<<validInput;
        return_periods_lineEdit->setText(validInput);
    }

    return validInput;
}


void HazardOccurrenceWidget::commitReturnPeriods()
{
    auto LEtext = return_periods_lineEdit->text();

    if(LEtext.isEmpty())
        return;

    auto inputStr = this->checkReturnPeriodsValid(LEtext);

    QList<int> returnperiodArray;

    // Remove any white space from the string
    inputStr.remove(" ");

    // Split the incoming text into the parts delimited by commas
    std::vector<std::string> subStringVec;

    // Create string stream from the string
    std::stringstream s_stream(inputStr.toStdString());

    // Split the input string to substrings at the comma
    while(s_stream.good())
    {
        std:: string subString;
        getline(s_stream, subString, ',');

        if(subString.empty())
            continue;

        subStringVec.push_back(subString);
    }

    for(auto&& subStr : subStringVec)
    {

        // Convert strings into numbers
        auto period = std::stod(subStr);

        returnperiodArray.push_back(period);
    }

    this->m_eqRupture->setReturnPeriods(returnperiodArray);
}


void HazardOccurrenceWidget::handleTypeChanged(const QString &val)
{
    if(val.compare("SA") == 0)
    {
        PeriodEdit->setEnabled(1);
        PeriodEdit->setStyleSheet("background-color: white");
    }
    else
    {
        PeriodEdit->setText("0.0");
        PeriodEdit->setDisabled(1);
        PeriodEdit->setStyleSheet("background-color: lightgray");
    }
}


void HazardOccurrenceWidget::loadHazardCurveFile()
{
    this->HazardCurveFile=QFileDialog::getOpenFileName(this,tr("Site File"));
    if(this->HazardCurveFile.isEmpty())
    {
        this->HazardCurveFile = "NULL";
        return;
    }
    this->setHCFile(this->HazardCurveFile);
}




