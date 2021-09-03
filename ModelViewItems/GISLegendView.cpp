#include "GISLegendView.h"

#include <QHeaderView>


GISLegendView::GISLegendView(QWidget *parent) : QTreeView(parent)
{
    this->setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->hide();
    this->setEditTriggers(EditTrigger::NoEditTriggers);
    this->setSelectionMode(SelectionMode::NoSelection);


    this->setStyleSheet("border: 1px solid transparent;"
                        "border-radius: 10px;"
                        "background:  rgba(255, 255, 255, 150);");

    this->setObjectName("legendView");
    this->header()->setObjectName("legendView");
}


QSize GISLegendView::sizeHint() const
{
    if(model() == nullptr)
        return QSize();

    if (model()->rowCount() == 0)
        return QSize(sizeHintForColumn(0)*1.15, 0);

    int nToShow = model()->rowCount()+1;

    auto widthLegend = sizeHintForColumn(0)*1.15;
    auto heightLegend = nToShow*sizeHintForRow(0)*1.2;


    return QSize(widthLegend,heightLegend);
}



void GISLegendView::clear(void)
{
    this->hide();
}


QIdentityProxyModel *GISLegendView::getProxyModel() const
{
    return nullptr;
}

