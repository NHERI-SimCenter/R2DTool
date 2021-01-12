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


void CustomListWidget::addItem(const QString item, QString model, const double weight)
{
    auto num = theListWidget->count();

    QString newItem = QString::number(num+1) + ". " + item + " - weight="+ QString::number(weight);

    new QListWidgetItem(newItem, theListWidget);

    ListOfModels.push_back(model);
    ListOfWeights.push_back(weight);
}


void CustomListWidget::addItem(const QString item)
{
    auto num = theListWidget->count();

    QString newItem = QString::number(num+1) + ". " + item;

    new QListWidgetItem(newItem, theListWidget);

    ListOfModels.push_back(item);
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
    ListOfModels.clear();
    ListOfWeights.clear();
}


int CustomListWidget::getNumberOfItems(void)
{
    return theListWidget->count();
}


QVariantList CustomListWidget::getListOfWeights() const
{
    return ListOfWeights;
}


QStringList CustomListWidget::getListOfModels() const
{
    return ListOfModels;
}

