#include "PopUpWidget.h"

#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>

PopUpWidget::PopUpWidget(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    this->setMinimumWidth(350);
    this->setMinimumHeight(350);

    theTabWidget = new QTabWidget(this);
    theTabWidget->setDocumentMode(true);

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setText(tr("Close"));
    closeButton->setMaximumWidth(150);

    mainLayout->addWidget(theTabWidget);
    mainLayout->addWidget(closeButton,Qt::AlignHCenter);

    connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
}

PopUpWidget::~PopUpWidget()
{

}


void PopUpWidget::addTab(QWidget *tab, const QString& label)
{
    theTabWidget->addTab(tab,label);
}
