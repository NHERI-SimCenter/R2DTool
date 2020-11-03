#ifndef TREEITEM_H
#define TREEITEM_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.12.2020

#include <QVariant>
#include <QObject>
#include <QVector>
#include <QModelIndex>

class QDialog;

class TreeItem : public QObject
{
    Q_OBJECT

public:
    explicit TreeItem(const QVector<QVariant> &data, TreeItem *parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);
    void removeChild(TreeItem *child);
    void removeChild(int row);

    TreeItem* child(int row);

    QStringList getActionList();

    int childCount() const;
    int columnCount() const;

    QVariant data(int column) const;
    int row() const;

    TreeItem* getParentItem();

    // 0 = unchecked
    // 1 = partially checked
    // 2 = checked
    int getState() const;

    void setState(int state);

    QVector<TreeItem*> getChildItems() const;

    void moveChild(int sourceRow, int destinationRow);

    TreeItem *findChild(QString name);

public slots:

    void changeOpacity();

    void handleChangeOpacity(int value);

signals:

void opacityChanged(const QString& layerName, const double opacity);

private:
    QVector<TreeItem*> vecChildItems;
    QVector<QVariant> itemData;
    TreeItem* parentItem;
    int currentState;

    QString itemName;

    QDialog* opacityDialog;
};


#endif // TREEITEM_H
