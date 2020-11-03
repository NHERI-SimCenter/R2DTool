#include "TreeViewStyle.h"

#include <QStyleOption>
#include <QPainter>
#include <QRect>

TreeViewStyle::TreeViewStyle(QStyle* style) :QProxyStyle(style)
{

}


void TreeViewStyle::drawPrimitive (PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
    if(element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull())
    {
        if(option->rect.height() == 0)
        {
            auto brush = QBrush(QColor(Qt::black));
            auto pen = QPen(brush, 3, Qt::SolidLine);

            QStyleOption opt(*option);

            opt.rect.setLeft(0);
            if (widget)
                opt.rect.setRight(widget->width());


            painter->setPen(pen);
            painter->drawLine(opt.rect.topLeft(), opt.rect.topRight());

            QProxyStyle::drawPrimitive(element, &opt, painter, widget);

        }

        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
