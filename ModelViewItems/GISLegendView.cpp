#include "GISLegendView.h"

#include <QGraphicsOpacityEffect>

GISLegendView::GISLegendView(QWidget *parent) : QListView(parent)
{
    //    this->setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->resize(100,100);
    this->hide();
    this->setEditTriggers(EditTrigger::NoEditTriggers);
    this->setSelectionMode(SelectionMode::NoSelection);


    this->setStyleSheet("background-color: yellow;"
                        "border: 1px solid transparent;"
                        "border-radius: 10px;"
                        "background:  rgba(255, 255, 255, 200);");

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


void GISLegendView::setModel(QAbstractItemModel *model)
{
    this->updateGeometry();

    QListView::setModel(model);

    int nToShow = model->rowCount();

    auto widthLegend = sizeHintForColumn(0)*1.1;
    auto heightLegend = nToShow*sizeHintForRow(0)*1.1;

    resize(widthLegend, heightLegend);
    viewport()->resize(widthLegend, heightLegend);
    resizeContents(widthLegend, heightLegend);

    this->updateGeometry();
}
