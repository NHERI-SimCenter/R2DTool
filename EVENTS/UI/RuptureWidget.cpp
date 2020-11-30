#include "RuptureWidget.h"

#include "PointSourceRuptureWidget.h"
#include "EarthquakeRuptureForecastWidget.h"
#include "PointSourceRupture.h"
#include "EarthquakeRuptureForecast.h"

#include <QVBoxLayout>
#include <QStackedWidget>

RuptureWidget::RuptureWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    QVBoxLayout* layout = new QVBoxLayout(this);

    theRootStackedWidget = new QStackedWidget(this);
    theRootStackedWidget->setContentsMargins(0,0,0,0);

    pointSourceWidget = new PointSourceRuptureWidget(this);
    erfWidget = new EarthquakeRuptureForecastWidget(this);

    theRootStackedWidget->addWidget(pointSourceWidget);
    theRootStackedWidget->addWidget(erfWidget);

    theRootStackedWidget->setCurrentWidget(erfWidget);

    ruptureGroupBox = new QGroupBox(tr("Earthquake Rupture"));
    QVBoxLayout* boxLayout = new QVBoxLayout(ruptureGroupBox);

    ruptureSelectionCombo = new QComboBox(this);

    ruptureSelectionCombo->addItem("Earthquake Rupture Forecast");
    ruptureSelectionCombo->addItem("Point Source");

    connect(ruptureSelectionCombo,&QComboBox::currentTextChanged,this,&RuptureWidget::handleSelectionChanged);

    boxLayout->addWidget(ruptureSelectionCombo);
    boxLayout->addWidget(theRootStackedWidget);

    layout->addWidget(ruptureGroupBox);

    this->setLayout(layout);


}

QJsonObject RuptureWidget::getJson(void)
{
    QJsonObject ruptureObj;

    if(ruptureSelectionCombo->currentText().compare("Point Source") == 0)
        ruptureObj = pointSourceWidget->getRuptureSource()->getJson();
    else if(ruptureSelectionCombo->currentText().compare("Earthquake Rupture Forecast") == 0)
        ruptureObj = erfWidget->getRuptureSource()->getJson();

    return ruptureObj;
}



void RuptureWidget::handleSelectionChanged(const QString& selection)
{
    if(selection.compare("Point Source") == 0)
        theRootStackedWidget->setCurrentWidget(pointSourceWidget);
    else if(selection.compare("Earthquake Rupture Forecast") == 0)
        theRootStackedWidget->setCurrentWidget(erfWidget);

}
