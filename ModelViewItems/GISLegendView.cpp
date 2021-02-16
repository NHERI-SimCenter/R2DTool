#include "GISLegendView.h"

#include <QHeaderView>


GISLegendView::GISLegendView(QWidget *parent) : QTreeView(parent)
{
    currModel = nullptr;

    //    this->setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->resize(100,100);
    this->hide();
    this->setEditTriggers(EditTrigger::NoEditTriggers);
    this->setSelectionMode(SelectionMode::NoSelection);


    this->setStyleSheet("border: 1px solid transparent;"
                        "border-radius: 10px;"
                        "background:  rgba(255, 255, 255, 150);");

    this->header()->setObjectName("legendView");

}


//QSize GISLegendView::sizeHint() const
//{
//    if(model() == nullptr)
//        return QSize();

//    if (model()->rowCount() == 0)
//        return QSize(sizeHintForColumn(0)*1.1, 0);

//    int nToShow = model()->rowCount();

//    auto widthLegend = sizeHintForColumn(0)*1.1;
//    auto heightLegend = nToShow*sizeHintForRow(0)*1.1;


//    return QSize(widthLegend,heightLegend);
//}

//QSize GISLegendView::minimumSizeHint() const
//{
//    if(model() == nullptr)
//        return QSize();

//    if (model()->rowCount() == 0)
//        return QSize(sizeHintForColumn(0)*1.1, 0);

//    int nToShow = model()->rowCount();

//    auto widthLegend = sizeHintForColumn(0)*1.1;
//    auto heightLegend = nToShow*sizeHintForRow(0)*1.1;


//    return QSize(widthLegend,heightLegend);
//}


void GISLegendView::setModel(QAbstractItemModel* model)
{
    if(currModel == model)
        return;

    QTreeView::setModel(model);

    int nToShow = model->rowCount();

    auto widthLegend = sizeHintForColumn(0);
    auto heightLegend = nToShow*sizeHintForRow(0);

    auto modelName = model->objectName();

    if(!listModels.empty())
    {
        if(listModels.contains(modelName))
        {
            widthLegend *= 1.0;
            heightLegend *= 1.3;
        }
    }
    else
    {
        widthLegend *= 1.15;
        heightLegend *= 1.45;

        listModels.append(modelName);
    }

    resize(widthLegend, heightLegend);

    this->updateGeometry();
}


void GISLegendView::clear(void)
{
    listModels.clear();
}

QAbstractItemModel *GISLegendView::getModel() const
{
    return currModel;
}
