#include "CustomListWidget.h"

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

CustomListWidget::CustomListWidget(QWidget *parent, QString headerText) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    headerLabel = new QLabel(headerText, this);
    headerLabel->setStyleSheet("font-weight: bold; color: black");

    theListWidget = new QListWidget(this);
    theListWidget->setMinimumWidth(300);
    theListWidget->setWordWrap(true);

    mainLayout->addWidget(headerLabel);
    mainLayout->addWidget(theListWidget);
}


void CustomListWidget::addItem(const QString item)
{
    new QListWidgetItem(item, theListWidget);
}


void CustomListWidget::removeItem(const QString item)
{
    QList<QListWidgetItem*> items = theListWidget->findItems(item, Qt::MatchExactly);

    for(auto&& it : items)
    {
        theListWidget->removeItemWidget(it);
        delete it;
    }
}


void CustomListWidget::clear(void)
{
    theListWidget->clear();
}


int CustomListWidget::getNumberOfItems(void)
{
    return theListWidget->count();
}

