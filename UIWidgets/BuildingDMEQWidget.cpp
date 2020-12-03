#include "BuildingDMEQWidget.h"
#include "sectiontitle.h"
#include "PelicunDLWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

BuildingDMEQWidget::BuildingDMEQWidget(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theCurrentWidget(0), theRandomVariablesContainer(theRandomVariableIW)
{
    QVBoxLayout *layout = new QVBoxLayout();

    // The selection part
    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setMinimumWidth(250);
    label->setText(QString("Engineering Demand Paramater Selection"));
    edpSelection = new QComboBox();
    edpSelection->addItem(tr("Pelicun"));
    edpSelection->setObjectName("DMSelectionComboBox");

    theSelectionLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theSelectionLayout->addItem(spacer);
    theSelectionLayout->addWidget(edpSelection,1);
    theSelectionLayout->addStretch(1);
    layout->addLayout(theSelectionLayout);

    // Create the stacked widget
    theStackedWidget = new QStackedWidget();

    // Create the individual widgets add to stacked widget
    thePelicunDLWidget = new PelicunDLWidget(this);
    theStackedWidget->addWidget(thePelicunDLWidget);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    layout->setMargin(0);

    connect(edpSelection, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(edpSelectionChanged(QString)));

    this->edpSelectionChanged("Pelicun");

}


BuildingDMEQWidget::~BuildingDMEQWidget()
{

}


bool BuildingDMEQWidget::outputToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputToJSON(jsonObject);
    return true;
}


bool BuildingDMEQWidget::inputFromJSON(QJsonObject &jsonObject) {

    if (theCurrentWidget != 0)
    {
        return theCurrentWidget->inputFromJSON(jsonObject);
    }
    else
    {
        emit sendErrorMessage("EDP_SElection no current EDP selected");
    }

    return false;
}


void BuildingDMEQWidget::edpSelectionChanged(const QString &arg1)
{
    //
    // switch stacked widgets depending on text
    // note type output in json and name in pull down are not the same and hence the ||
    //

    if (arg1 == "Pelicun")
    {
        theStackedWidget->setCurrentIndex(0);
        theCurrentWidget = thePelicunDLWidget;
    }
    else
    {
        qDebug() << "ERROR ..  BuildingDMEQWidget selection .. type unknown: " << arg1;
        theCurrentWidget = nullptr;
    }
}


bool BuildingDMEQWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputAppDataToJSON(jsonObject);
    return true;
}


bool BuildingDMEQWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
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
        emit sendErrorMessage(" BuildingDMEQWidget - no Application key found");
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
        emit sendErrorMessage("BuildingDMEQWidget - no valid type found");
        return false;
    }

    edpSelection->setCurrentIndex(index);

    // invoke inputAppDataFromJSON on new type
    if (theCurrentWidget != 0)
    {
        return theCurrentWidget->inputAppDataFromJSON(jsonObject);
    }

    return true;
}

bool BuildingDMEQWidget::copyFiles(QString &destDir) {

    if (theCurrentWidget != nullptr)
    {
        return  theCurrentWidget->copyFiles(destDir);
    }

    return false;
}
