#include "RecordSelectionWidget.h"


RecordSelectionWidget::RecordSelectionWidget(RecordSelectionConfig& selectionConfig, QWidget *parent) : QWidget(parent), m_selectionConfig(selectionConfig)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* selectionGroupBox = new QGroupBox(this);
    selectionGroupBox->setTitle("Record Selection");
    selectionGroupBox->setContentsMargins(0,0,0,0);

    //selectionGroupBox->setMinimumWidth(400);
    //selectionGroupBox->setMaximumWidth(500);

    QGridLayout* formLayout = new QGridLayout(selectionGroupBox);

    QLabel* databaseLabel = new QLabel(tr("Database:"),this);
    m_dbBox = new QComboBox(this);
    m_dbBox->addItem("PEER NGA West 2");
    connect(this->m_dbBox, &QComboBox::currentTextChanged, &this->m_selectionConfig, &RecordSelectionConfig::setDatabase);
    m_dbBox->setCurrentText("PEER NGA West 2");
    m_selectionConfig.setDatabase("PEER NGA West 2");
    m_dbBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

//    m_errorBox = new QComboBox(this);

//    QLabel* errorLabel = new QLabel(tr("Error Metric:"),this);
//    m_errorBox->addItem("Absolute Sum", RecordSelectionConfig::ErrorMetric::AbsSum);
//    m_errorBox->addItem("Root Mean Square", RecordSelectionConfig::ErrorMetric::RMSE);
//    m_errorBox->addItem("Mean Square", RecordSelectionConfig::ErrorMetric::MSE);
//    m_errorBox->addItem("Mean Absolute Percent Error", RecordSelectionConfig::ErrorMetric::MAPE);
//    m_errorBox->setCurrentIndex(m_errorBox->findData(this->m_selectionConfig.getError()));
//    m_errorBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    formLayout->addWidget(databaseLabel,0,0);
    formLayout->addWidget(m_dbBox,0,1);
//    formLayout->addWidget(errorLabel,1,0);
//    formLayout->addWidget(m_errorBox,1,1);

    selectionGroupBox->setLayout(formLayout);

    layout->addWidget(selectionGroupBox);

    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}


