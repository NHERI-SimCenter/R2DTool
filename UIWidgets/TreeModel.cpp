#include "TreeModel.h"
#include "TreeItem.h"

#include <QStringList>
#include <QMimeData>
#include <QDataStream>
#include <QDebug>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{    
    rootItem = new TreeItem({tr("Layers")});

}


TreeModel::~TreeModel()
{
    delete rootItem;
}


int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();

    return rootItem->columnCount();
}


Qt::DropActions TreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


bool TreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if(action != Qt::MoveAction)
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/data"))
        return false;

    if (column != 0)
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("application/data");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;

    int numItems = 0;
    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        newItems << text;
        ++numItems;
    }


    // Because the item row numbers are relative to the parent, need to do a deep search to find the model index of the item
    std::function<QModelIndex(TreeItem*)> getItemIndexDeepSearch = [&](TreeItem* fromItem)
    {
        auto itemParent = fromItem->getParentItem();

        // The item row, which is relative to the parent
        auto itemParentRow = fromItem->row();

        if(itemParent != rootItem)
        {
            auto itemRow = fromItem->row();

            // Get the index of the parent
            QModelIndex parentIndex =  this->index(itemRow, 0, getItemIndexDeepSearch(itemParent));

            return parentIndex;
        }
        else
        {
            return this->index(itemParentRow,0);
        }
    };


    foreach (const QString &itemName, newItems)
    {
        auto fromItem = rootItem->findChild(itemName);

        if(fromItem == nullptr)
            return false;

        auto itemIndex = getItemIndexDeepSearch(fromItem);

        // This row is relative to the parent!
        auto sourceRow = fromItem->row();

        // ModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild
        moveRow(itemIndex, sourceRow, itemIndex, beginRow);

    }

    return true;
}


QStringList TreeModel::mimeTypes() const
{
    return QStringList("application/data");
}


QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if(item->getState() == 0)
            return Qt::Unchecked;
        else if(item->getState() == 1)
            return Qt::PartiallyChecked;
        else if(item->getState() == 2)
            return Qt::Checked;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    //    if (!index.isValid())
    //        return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled ;

    Qt::ItemFlags flags = Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;

    if (index.column() == 0)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->getParentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}


TreeItem *TreeModel::getRootItem() const
{
    return rootItem;
}


bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    auto valueAsInt = value.toInt();

    std::function<void(TreeItem*, const int)> nestedItemChecker = [&](TreeItem* item, const int value)
    {
        auto children = item->getChildItems();

        for(auto&& it : children)
        {
            nestedItemChecker(it,value);

            emit itemValueChanged(it);
        }

        item->setState(value);

        emit itemValueChanged(item);

        // Set the parent as a partial checked
        auto parent = item->getParentItem();

        if(parent)
            parent->setState(1);

    };

    if (index.column() == 0 && role == Qt::CheckStateRole)
    {
        nestedItemChecker(item,valueAsInt);

        emit dataChanged(QModelIndex(), QModelIndex());

        return true;
    }
    else
        return false;

    return QAbstractItemModel::setData(index, value, role);
}


TreeItem* TreeModel::addItemToTree(const QString itemText, const QString layerID, TreeItem* parent)
{
    if(parent == nullptr)
        parent = rootItem;

    if(auto exists = this->getTreeItem(itemText, parent))
        return exists;

    QVector<QVariant> childText = {itemText};

    auto childItem = new TreeItem(childText, layerID, parent);

    parent->appendChild(childItem);

    emit layoutChanged();

    return childItem;
}


bool TreeModel::removeItemFromTree(const QString& itemName)
{
    std::function<bool(TreeItem*, const QString&)> nestedDeleter = [&](TreeItem* item, const QString& name)
    {
        QVector<TreeItem *> children = item->getChildItems();

        auto index = -1;
        for(int i = 0; i<children.size(); ++i)
        {
            auto child = children.at(i);

            auto childName = child->data(0).toString();

            if(name.compare(childName) == 0)
            {
                index = i;
                break;
            }

            if(nestedDeleter(child,name))
                return true;
        }

        if(index > -1)
        {
            item->removeChild(index);
            return true;
        }

        return false;

    };

    auto res = nestedDeleter(rootItem, itemName);

    emit layoutChanged();

    return res;
}

TreeItem* TreeModel::getTreeItem(const QString& itemName, const TreeItem* parent) const
{
    QString parentName;

    if(parent != nullptr)
    {
        parentName = parent->getName();
    }

    return this->getTreeItem(itemName,parentName);
}


TreeItem* TreeModel::getTreeItem(const QString& itemName, const QString& parentName) const
{

    std::function<TreeItem* (TreeItem*, const QString&, const QString&)> nestedItemFinder = [&](TreeItem* item, const QString& name, const QString& parentName) -> TreeItem*
    {
        auto thisItemParent = item->getParentItem();

        // Check if this is the item
        auto thisItemName = item->data(0).toString();
        if(name.compare(thisItemName) == 0 )
        {
            auto thisItemParentName = thisItemParent->getName();
            if(thisItemParent == rootItem || parentName.compare(thisItemParentName) == 0)
                return item;
        }


        // Now check the children of this item
        QVector<TreeItem *> children = item->getChildItems();
        for(int i = 0; i<children.size(); ++i)
        {
            auto child = children.at(i);

            // Check the childrens children
            if(auto foundChild = nestedItemFinder(child,name, parentName))
                return foundChild;
        }

        // Return a null item if none is found
        return nullptr;
    };

    return nestedItemFinder(rootItem, itemName, parentName);
}


QMimeData* TreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("application/data", encodedData);
    return mimeData;
}


bool TreeModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    Q_UNUSED(count);

    auto sourceName = sourceParent.data(0).toString();
    auto fromItem = rootItem->findChild(sourceName)->getParentItem();

    if(sourceName.isEmpty() || fromItem == nullptr)
        return false;

    // const QModelIndex &sourceParent, int sourceFirst, int sourceLast, const QModelIndex &destinationParent, int destinationChild
    if (!beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild))
        return false;

    // Need if moving up need to decrement the destination index
    if(sourceRow <= destinationChild)
        destinationChild--;

    fromItem->moveChild(sourceRow,destinationChild);

    endMoveRows();

    emit rowPositionChanged(sourceRow, destinationChild);

    return true;
}
