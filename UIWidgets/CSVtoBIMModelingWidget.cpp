#include "CSVtoBIMModelingWidget.h"
#include "ComponentInputWidget.h"
#include "WorkflowAppRDT.h"
#include "AssetsWidget.h"
#include "CSVReaderWriter.h"

#include <QJsonObject>
#include <QGroupBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QRegExpValidator>

CSVtoBIMModelingWidget::CSVtoBIMModelingWidget(QWidget *parent) : SimCenterAppWidget(parent)
{    

    QGridLayout* gridLayout = new QGridLayout(this);

    QLabel* IDLabel = new QLabel("Select a range of building IDs for which to create a model. By default, all buildings selected for analysis are converted into a BIM.",this);

    selectComponentsLineEdit = new QLineEdit();
    selectComponentsLineEdit->setMaximumWidth(1000);
    selectComponentsLineEdit->setMinimumWidth(400);
    selectComponentsLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    selectComponentsLineEdit->setPlaceholderText("e.g., 1, 3, 5-10, 12");

    // Create a regExp validator to make sure only '-' & ',' & ' ' & numbers are input
    QRegExp LERegExp ("((([1-9][0-9]*)|([1-9][0-9]*-[1-9][0-9]*))[ ]*,[ ]*)*([[1-9][0-9]*-[1-9][0-9]*|[1-9][0-9]*)");
    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);
    selectComponentsLineEdit->setValidator(LEValidator);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addWidget(IDLabel,0,0);
    gridLayout->addWidget(selectComponentsLineEdit,1,0);
    gridLayout->addItem(vspacer,2,0,1,2);

}

bool CSVtoBIMModelingWidget::outputToJSON(QJsonObject &jsonObj)
{

    jsonObj.insert("Application", "CSV_to_BIM");

    auto mainWindowWidget = WorkflowAppRDT::getInstance();

    auto assetsWidget = mainWindowWidget->getAssetsWidget();

    auto buildingSourceFile = assetsWidget->getBuildingWidget()->getPathToComponentFile();

    QJsonObject appDataObj;
    appDataObj.insert("Min", 1);
    appDataObj.insert("Max", 3);
    appDataObj.insert("buildingSourceFile", buildingSourceFile);

    jsonObj.insert("ApplicationData",appDataObj);

    return true;
}



CSVtoBIMModelingWidget::~CSVtoBIMModelingWidget()
{

}









