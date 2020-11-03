#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include <QDialog>
#include <QPointer>

class QTabWidget;

class PopUpWidget : public QDialog
{
    Q_OBJECT
public:
    explicit PopUpWidget(QWidget *parent = nullptr);

    ~PopUpWidget();

    void addTab(QWidget *tab, const QString& label);

signals:

private:

QPointer<QTabWidget> theTabWidget;

};

#endif // POPUPWIDGET_H
