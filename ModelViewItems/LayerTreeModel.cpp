/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "LayerTreeItem.h"
#include "LayerTreeModel.h"

#include <QDataStream>
#include <QDebug>
#include <QMimeData>
#include <QStringList>

LayerTreeModel::LayerTreeModel(QObject *parent) : QAbstractItemModel(parent)
{    
    rootItem = new LayerTreeItem({tr("Layers")},0);
}


LayerTreeModel::~LayerTreeModel()
{
    delete rootItem;
}


int LayerTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<LayerTreeItem*>(parent.internalPointer())->columnCount();

    return rootItem->columnCount();
}


Qt::DropActions LayerTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


Qt::DropActions LayerTreeModel::supportedDragActions() const
{
    return Qt::MoveAction;
}


QStringList LayerTreeModel::mimeTypes() const
{
    return QStringList("application/data");
}


QMimeData* LayerTreeModel::mimeData(const QModelIndexList &indexes) const
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


bool LayerTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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


QVariant LayerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    LayerTreeItem *item = static_cast<LayerTreeItem*>(index.internalPointer());

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


Qt::ItemFlags LayerTreeModel::flags(const QModelIndex &index) const
{

    Qt::ItemFlags flags = Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled ;

    if (index.column() == 0)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}


QVariant LayerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex LayerTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    LayerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LayerTreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex LayerTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    LayerTreeItem *childItem = static_cast<LayerTreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->getParentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int LayerTreeModel::rowCount(const QModelIndex &parent) const
{
    LayerTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LayerTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}


LayerTreeItem *LayerTreeModel::getRootItem() const
{
    return rootItem;
}


bool LayerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    LayerTreeItem *item = static_cast<LayerTreeItem*>(index.internalPointer());

    auto valueAsInt = value.toInt();

    std::function<void(LayerTreeItem*, const int)> nestedItemChecker = [&](LayerTreeItem* item, const int value)
    {
        auto children = item->getChildItems();

        for(auto&& it : children)
        {
            auto child = static_cast<LayerTreeItem*>(it);
            nestedItemChecker(child,value);

            emit itemValueChanged(child);
        }

        item->setState(value);

        emit itemValueChanged(item);

        // Set the parent as a partial checked
        auto parent = static_cast<LayerTreeItem*>(item->getParentItem());

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


LayerTreeItem* LayerTreeModel::addItemToTree(const QString itemText, const QString layerID, LayerTreeItem* parent)
{
    if(parent == nullptr)
        parent = rootItem;

    if(auto exists = this->getLayerTreeItem(itemText, parent))
        return exists;

    QVector<QVariant> childText = {itemText};

    auto childItem = new LayerTreeItem(childText, layerID, parent);

    parent->appendChild(childItem);

    emit layoutChanged();

    return childItem;
}


bool LayerTreeModel::removeItemFromTree(const QString& itemID)
{
    std::function<bool(TreeItem*, const QString&)> nestedDeleter = [&](TreeItem* item, const QString& ID)
    {
        QVector<TreeItem *> children = item->getChildItems();

        auto index = -1;
        for(int i = 0; i<children.size(); ++i)
        {
            auto child = children.at(i);
            auto childID = child->getItemID();

            if(ID.compare(childID) == 0)
            {
                index = i;
                break;
            }

            if(nestedDeleter(child,ID))
                return true;
        }

        if(index > -1)
        {
            item->removeChild(index);
            return true;
        }

        return false;

    };

    auto res = nestedDeleter(rootItem, itemID);

    emit layoutChanged();

    return res;
}


LayerTreeItem* LayerTreeModel::getLayerTreeItem(const QString& itemName, const LayerTreeItem* parent) const
{
    QString parentName;

    if(parent != nullptr)
    {
        parentName = parent->getName();
    }

    return this->getLayerTreeItem(itemName,parentName);
}


LayerTreeItem* LayerTreeModel::getLayerTreeItem(const QString& itemName, const QString& parentName) const
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

    return static_cast<LayerTreeItem*>(nestedItemFinder(rootItem, itemName, parentName));
}


bool LayerTreeModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
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


bool LayerTreeModel::clear(void)
{
    auto children = rootItem->getChildItems();

    for(auto&& child : children)
    {
        auto res = this->removeItemFromTree(child->getItemID());

        if(res == false)
            return false;
    }

    return true;
}

