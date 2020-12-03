#include "BuildingEDPEQWidget.h"
#include "sectiontitle.h"
#include "StandardEarthquakeEDP.h"
#include "UserDefinedEDPR.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

BuildingEDPEQWidget::BuildingEDPEQWidget(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theCurrentWidget(0), theRandomVariablesContainer(theRandomVariableIW)
{
    QVBoxLayout *layout = new QVBoxLayout();

    // the selection part
    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setMinimumWidth(250);
    label->setText(QString("Engineering Demand Paramater Selection"));
    edpSelection = new QComboBox();
    edpSelection->addItem(tr("Standard Earthquake"));
    edpSelection->addItem(tr("User Defined"));
    edpSelection->setObjectName("EDPSelectionComboBox");

    theSelectionLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theSelectionLayout->addItem(spacer);
    theSelectionLayout->addWidget(edpSelection,1);
    theSelectionLayout->addStretch(1);
    layout->addLayout(theSelectionLayout);

    // Create the stacked widget
    theStackedWidget = new QStackedWidget();


    // Create the individual widgets add to stacked widget
    theStandardEarthquakeEDPs = new StandardEarthquakeEDP(theRandomVariablesContainer);
    theStackedWidget->addWidget(theStandardEarthquakeEDPs);

    theUserDefinedEDPs = new UserDefinedEDPR(theRandomVariablesContainer);
    theStackedWidget->addWidget(theUserDefinedEDPs);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    theCurrentWidget=theStandardEarthquakeEDPs;
    layout->setMargin(0);

    connect(edpSelection, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(edpSelectionChanged(QString)));


    edpSelection->setCurrentText("User Defined");
    theUserDefinedEDPs->setEDPSpecsFile("/Users/steve/Desktop/SimCenter/Examples/rWhaleExample/input_data_rdt/EDP_specs.json");
}


BuildingEDPEQWidget::~BuildingEDPEQWidget()
{

}


bool BuildingEDPEQWidget::outputToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputToJSON(jsonObject);
    return true;
}


bool BuildingEDPEQWidget::inputFromJSON(QJsonObject &jsonObject)
{

    if (theCurrentWidget != nullptr)
    {
        return theCurrentWidget->inputFromJSON(jsonObject);
    }
    else
    {
        emit sendErrorMessage("EDP_SElection no current EDP selected");
    }

    return false;
}


void BuildingEDPEQWidget::edpSelectionChanged(const QString &arg1)
{

    // switch stacked widgets depending on text
    // note type output in json and name in pull down are not the same and hence the ||

    if (arg1 == "Standard Earthquake")
    {
        theStackedWidget->setCurrentIndex(0);
        theCurrentWidget = theStandardEarthquakeEDPs;
    }
    else if(arg1 == "User Defined")
    {
        theStackedWidget->setCurrentIndex(1);
        theCurrentWidget = theUserDefinedEDPs;
    }

    else
    {
        qDebug() << "ERROR ..  BuildingEDPEQWidget selection .. type unknown: " << arg1;
    }
}

bool BuildingEDPEQWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputAppDataToJSON(jsonObject);
    return true;
}


bool BuildingEDPEQWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // get name from "Application" key

    QString type;
    if (jsonObject.contains("Application"))
    {
        QJsonValue theName = jsonObject["Application"];
        type = theName.toString();
    }
    else
    {
        emit sendErrorMessage(" BuildingEDPEQWidget - no Application key found");
        return false;
    }

    // based on application name value set edp type
    int index = 0;
    if ((type == QString("Standard Earthquake EDPs")) ||
            (type == QString("StandardEarthquakeEDP")))
    {
        index = 0;
    } else if ((type == QString("UserDefinedEDP")) ||
               (type == QString("User Defined EDPs")))
    {
        index = 1;
    }
    else
    {
        emit sendErrorMessage(" BuildingEDPEQWidget - no valid type found");
        return false;
    }

    edpSelection->setCurrentIndex(index);

    // invoke inputAppDataFromJSON on new type

    if (theCurrentWidget != nullptr)
    {
        return theCurrentWidget->inputAppDataFromJSON(jsonObject);
    }

    return true;
}

bool BuildingEDPEQWidget::copyFiles(QString &destDir) {

    if (theCurrentWidget != nullptr)
    {
        return  theCurrentWidget->copyFiles(destDir);
    }

    return false;
}
