#include "PelicunDLWidget.h"

#include <QVBoxLayout>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QGridLayout>

PelicunDLWidget::PelicunDLWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* gmpeGroupBox = new QGroupBox(this);
    gmpeGroupBox->setTitle("Pelicun Damage and Loss Prediction Methodology");

    QGridLayout* gridLayout = new QGridLayout(gmpeGroupBox);

    QLabel* typeLabel = new QLabel(tr("Damage and Loss Method:"),this);
    DLTypeComboBox = new QComboBox(this);
    DLTypeComboBox->addItem("HAZUS MH EQ");
    DLTypeComboBox->addItem("HAZUS MH EQ IM");
    DLTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QLabel* realizationsLabel = new QLabel(tr("Number of realizations:"),this);
    realizationsLineEdit = new QLineEdit(this);

    QIntValidator* validator = new QIntValidator(this);
    validator->setBottom(0);
    realizationsLineEdit->setValidator(validator);
    realizationsLineEdit->setText("5");

    QLabel* eventTimeLabel = new QLabel(tr("Event time:"),this);
    eventTimeComboBox = new QComboBox(this);
    eventTimeComboBox->addItem("on");
    eventTimeComboBox->addItem("off");
    eventTimeComboBox->setCurrentIndex(1);

    detailedResultsCheckBox = new QCheckBox("Output detailed results",this);
    logFileCheckBox = new QCheckBox("Log file",this);
    logFileCheckBox->setChecked(true);
    coupledEDPCheckBox = new QCheckBox("Coupled EDP",this);
    groundFailureCheckBox= new QCheckBox("Include ground failure",this);

    auto Vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addWidget(typeLabel,0,0);
    gridLayout->addWidget(DLTypeComboBox,0,1);
    gridLayout->addWidget(eventTimeLabel,1,0);
    gridLayout->addWidget(eventTimeComboBox,1,1);
    gridLayout->addWidget(realizationsLabel,2,0);
    gridLayout->addWidget(realizationsLineEdit,2,1);

    gridLayout->addWidget(detailedResultsCheckBox,3,0);
    gridLayout->addWidget(logFileCheckBox,4,0);
    gridLayout->addWidget(coupledEDPCheckBox,5,0);
    gridLayout->addWidget(groundFailureCheckBox,6,0);
    gridLayout->addItem(Vspacer,7,0,1,2);

    layout->addWidget(gmpeGroupBox);
    this->setLayout(layout);
}


bool PelicunDLWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject.insert("Application","pelicun");

    QJsonObject appDataObj;

    appDataObj.insert("DL_Method",DLTypeComboBox->currentText());
    appDataObj.insert("Realizations",realizationsLineEdit->text().toInt());
    appDataObj.insert("detailed_results",detailedResultsCheckBox->isChecked());
    appDataObj.insert("log_file",logFileCheckBox->isChecked());
    appDataObj.insert("coupled_EDP",coupledEDPCheckBox->isChecked());
    appDataObj.insert("event_time",eventTimeComboBox->currentText());
    appDataObj.insert("ground_failure",groundFailureCheckBox->isChecked());

    jsonObject.insert("ApplicationData",appDataObj);

    return true;
}


bool PelicunDLWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("DL_Method"))
            DLTypeComboBox->setCurrentText(appData["DL_Method"].toString());

        if (appData.contains("Realizations"))
            realizationsLineEdit->setText(QString::number(appData["Realizations"].toInt()));

        if (appData.contains("coupled_EDP"))
            coupledEDPCheckBox->setChecked(appData["coupled_EDP"].toBool());

        if (appData.contains("detailed_results"))
            detailedResultsCheckBox->setChecked(appData["detailed_results"].toBool());

        if (appData.contains("ground_failure"))
            groundFailureCheckBox->setChecked(appData["ground_failure"].toBool());

        if (appData.contains("log_file"))
            logFileCheckBox->setChecked(appData["log_file"].toBool());

        if (appData.contains("event_time"))
            eventTimeComboBox->setCurrentText(appData["event_time"].toString());

    }

    return true;
}


