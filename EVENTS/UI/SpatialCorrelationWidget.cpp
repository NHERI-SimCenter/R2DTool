#include "SpatialCorrelationWidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>

SpatialCorrelationWidget::SpatialCorrelationWidget(QWidget *parent): QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* spatCorrGroupBox = new QGroupBox(this);
    spatCorrGroupBox->setTitle("Spatial Correlation and Scaling");
    spatCorrGroupBox->setContentsMargins(0,0,0,0);
    //FMK spatCorrGroupBox->setMinimumWidth(400);
    //FMK spatCorrGroupBox->setMaximumWidth(500);

    QGridLayout* gridLayout = new QGridLayout(spatCorrGroupBox);
    spatCorrGroupBox->setLayout(gridLayout);

    QLabel* spatialCorrelationInterLabel = new QLabel(tr("Inter-event\nSpatial Correlation Model:"),this);

    m_correlationBoxInter = new QComboBox(this);
    m_correlationBoxInter->addItem("Baker & Jayaram (2008)");

    m_correlationBoxInter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QLabel* spatialCorrelationIntraLabel = new QLabel(tr("Intra-event\nSpatial Correlation Model:"),this);

    m_correlationBoxIntra = new QComboBox(this);
    m_correlationBoxIntra->addItem("Markhvida et al. (2017)");
    m_correlationBoxIntra->addItem("Jayaram & Baker (2009)");
    m_correlationBoxIntra->addItem("Loth & Baker (2013)");
    m_correlationBoxIntra->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QLabel* minScalingLabel = new QLabel(tr("Minimum Scaling Factor:"),this);
    QLabel* maxScalingLabel = new QLabel(tr("Maximum Scaling Factor:"),this);

    QDoubleValidator* doubleValid = new QDoubleValidator(0.0, 100.0,2,this);

    minScalingLineEdit = new QLineEdit(this);
    minScalingLineEdit->setText("0.1");

    maxScalingLineEdit = new QLineEdit(this);
    maxScalingLineEdit->setText("20.0");

    minScalingLineEdit->setValidator(doubleValid);
    maxScalingLineEdit->setValidator(doubleValid);

    auto smallVSpacer = new QSpacerItem(0,20);

    auto Vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout->addItem(Vspacer,0,0,1,2);
    gridLayout->addWidget(spatialCorrelationInterLabel,1,0);
    gridLayout->addWidget(m_correlationBoxInter,1,1);
    gridLayout->addWidget(spatialCorrelationIntraLabel,2,0);
    gridLayout->addWidget(m_correlationBoxIntra,2,1);
    gridLayout->addItem(smallVSpacer,3,0,1,2);
    gridLayout->addWidget(minScalingLabel,4,0);
    gridLayout->addWidget(minScalingLineEdit,4,1);
    gridLayout->addWidget(maxScalingLabel,5,0);
    gridLayout->addWidget(maxScalingLineEdit,5,1);
    gridLayout->addItem(Vspacer,6,0,1,2);

    layout->addWidget(spatCorrGroupBox);
    this->setLayout(layout);

    m_correlationBoxInter->setCurrentIndex(0);
}


QJsonObject SpatialCorrelationWidget::getJsonCorr()
{
    QJsonObject spatCorr;
    spatCorr.insert("SaInterEvent", m_correlationBoxInter->currentText());
    spatCorr.insert("SaIntraEvent", m_correlationBoxIntra->currentText());

    return spatCorr;
}

QJsonObject SpatialCorrelationWidget::getJsonScaling()
{
    QJsonObject scaling;
    scaling.insert("Maximum", maxScalingLineEdit->text().toDouble());
    scaling.insert("Minimum", minScalingLineEdit->text().toDouble());

    return scaling;
}

