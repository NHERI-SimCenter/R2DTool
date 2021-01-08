#ifndef CustomListWidget_H
#define CustomListWidget_H

#include <QListWidget>

class QLabel;

class CustomListWidget : public QWidget
{
public:
    CustomListWidget(QWidget *parent = nullptr, QString headerText = QString());

    void clear(void);

public slots:

    void addItem(const QString item);

    void removeItem(const QString item);

    int getNumberOfItems(void);

private:

    QListWidget* theListWidget;
    QLabel* headerLabel;
};

#endif // CustomListWidget_H
