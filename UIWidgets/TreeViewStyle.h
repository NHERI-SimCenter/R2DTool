#ifndef TREEVIEWSTYLE_H
#define TREEVIEWSTYLE_H

#include <QProxyStyle>

class TreeViewStyle: public QProxyStyle
{
public:
    TreeViewStyle(QStyle* style = nullptr);

    void drawPrimitive (PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = nullptr) const;
};


#endif // TREEVIEWSTYLE_H
