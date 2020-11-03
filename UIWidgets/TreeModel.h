#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int col = 0, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    TreeItem *getRootItem() const;

    // If parent item is not provided, the item will get added to the root of the tree
    TreeItem* addItemToTree(const QString itemText, TreeItem* parent = nullptr);

    bool removeItemFromTree(const QString& itemName);

    TreeItem *getTreeItem(const QString& itemName) const;

    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes () const override;

    QMimeData* mimeData(const QModelIndexList &indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    bool moveRows(const QModelIndex &srcParent, int srcRow, int count, const QModelIndex &dstParent, int dstChild) override;

signals:

    void itemValueChanged(TreeItem* item);

    void rowPositionChanged(const int oldPos, const int newPos);

private:
    TreeItem *rootItem;
};

#endif // TREEMODEL_H
