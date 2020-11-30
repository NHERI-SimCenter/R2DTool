#include "HBoxFormLayout.h"
#include <QLabel>

HBoxFormLayout::HBoxFormLayout():QHBoxLayout()
{

}

HBoxFormLayout::HBoxFormLayout(QWidget *parent):QHBoxLayout(parent)
{

}

void HBoxFormLayout::addField(const QString &labelText, QWidget *widget)
{
    QLabel* label = new QLabel(labelText);
    this->addWidget(label, 0);
    this->addWidget(widget, 1);
    this->addSpacing(10);
}
