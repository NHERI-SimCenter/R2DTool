#ifndef COLORDIALOGDELEGATE_H
#define COLORDIALOGDELEGATE_H

#include <QStyledItemDelegate>

class QColorDialog;

class ColorDialogDelegate : public QStyledItemDelegate
{
public:
    ColorDialogDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QColor getSingleColor(const QColor& initialColor);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;


private:
    QColorDialog* colorDialog;
};

#endif // COLORDIALOGDELEGATE_H
