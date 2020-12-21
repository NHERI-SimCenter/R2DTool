#include "GMPEWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

GMPEWidget::GMPEWidget(GMPE& gmpe, QWidget *parent): QWidget(parent), m_gmpe(gmpe)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* gmpeGroupBox = new QGroupBox(this);
    gmpeGroupBox->setTitle("Ground Motion Prediction Equation");

    QHBoxLayout* formLayout = new QHBoxLayout(gmpeGroupBox);
    m_typeBox = new QComboBox(this);

    m_typeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QLabel* typeLabel = new QLabel(tr("Type:"),this);

    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeBox);
//    formLayout->addStretch(1);

    gmpeGroupBox->setLayout(formLayout);
    //FMK gmpeGroupBox->setMinimumWidth(400);
    //FMK gmpeGroupBox->setMaximumWidth(500);

    layout->addWidget(gmpeGroupBox);
    this->setLayout(layout);

    const QStringList validType = this->m_gmpe.validTypes();

    QStringListModel* typeModel = new QStringListModel(validType);
    m_typeBox->setModel(typeModel);
    m_typeBox->setCurrentIndex(validType.indexOf(m_gmpe.type()));
    this->setupConnections();
}

void GMPEWidget::setupConnections()
{
    connect(this->m_typeBox, &QComboBox::currentTextChanged,
            &this->m_gmpe, &GMPE::setType);

    connect(&this->m_gmpe, &GMPE::typeChanged,
            this->m_typeBox, &QComboBox::setCurrentText);
}
