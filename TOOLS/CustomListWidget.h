#ifndef CustomListWidget_H
#define CustomListWidget_H

#include <QListWidget>

class QLabel;

class CustomListWidget : public QWidget
{
public:
    CustomListWidget(QWidget *parent = nullptr, QString headerText = QString());

    void clear(void);

    QStringList getListOfModels() const;

    QVariantList getListOfWeights() const;

public slots:

    void addItem(const QString item, QString model, const double weight);
    void addItem(const QString item);

    void removeItem(const QString item);

    int getNumberOfItems(void);

private:

    QStringList ListOfModels;
    QVariantList ListOfWeights;
    QListWidget* theListWidget;
    QLabel* headerLabel;
};

#endif // CustomListWidget_H
