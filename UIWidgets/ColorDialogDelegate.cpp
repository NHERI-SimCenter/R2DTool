#include "ColorDialogDelegate.h"

#include <QColorDialog>
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

ColorDialogDelegate::ColorDialogDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    colorDialog = new QColorDialog();
}


QWidget *ColorDialogDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QWidget *editor = new QWidget(parent);

    return editor;
}


void ColorDialogDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


bool ColorDialogDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{

    if(event->type()==QEvent::MouseButtonPress)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(me->button() == Qt::LeftButton )
        {
            auto currCol =  model->data(index,Qt::BackgroundRole).value<QColor>();

            auto newColor = this->getSingleColor(currCol);

            model->setData(index, newColor, Qt::EditRole);
        }
    }

    return QStyledItemDelegate::editorEvent(event,model,option,index);
}


QColor ColorDialogDelegate::getSingleColor(const QColor& initialColor)
{
    colorDialog->setCurrentColor(initialColor);

    auto res = colorDialog->exec();

    if(res == 0)
        return QColor();

    return colorDialog->currentColor();
}



