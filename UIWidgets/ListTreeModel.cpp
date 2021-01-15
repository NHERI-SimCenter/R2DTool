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

#include "TreeItem.h"
#include "ListTreeModel.h"

#include <QDataStream>
#include <QDebug>
#include <QMimeData>
#include <QStringList>

ListTreeModel::ListTreeModel(QString headerText, QObject *parent) : QAbstractItemModel(parent)
{    
    rootItem = new TreeItem({headerText});
}


ListTreeModel::~ListTreeModel()
{
    delete rootItem;
}


int ListTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();

    return rootItem->columnCount();
}


QVariant ListTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}


Qt::ItemFlags ListTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}


QVariant ListTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex ListTreeModel::index(int row, int column, const QModelIndex &parent) const
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


QModelIndex ListTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->getParentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int ListTreeModel::rowCount(const QModelIndex &parent) const
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


TreeItem *ListTreeModel::getRootItem() const
{
    return rootItem;
}


bool ListTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    return QAbstractItemModel::setData(index, value, role);
}


TreeItem* ListTreeModel::addItemToTree(const QString itemText, TreeItem* parent)
{
    if(parent == nullptr)
        parent = rootItem;

    if(auto exists = this->getTreeItem(itemText, parent))
        return exists;

    QVector<QVariant> childText = {itemText};

    auto childItem = new TreeItem(childText, parent);

    parent->appendChild(childItem);

    emit layoutChanged();

    return childItem;
}


bool ListTreeModel::removeItemFromTree(const QString& itemName)
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


TreeItem* ListTreeModel::getTreeItem(const QString& itemName, const TreeItem* parent) const
{
    QString parentName;

    if(parent != nullptr)
    {
        parentName = parent->getName();
    }

    return this->getTreeItem(itemName,parentName);
}


TreeItem* ListTreeModel::getTreeItem(const QString& itemName, const QString& parentName) const
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


bool ListTreeModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
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


bool ListTreeModel::clear(void)
{
    auto children = rootItem->getChildItems();

    for(auto&& child : children)
    {
        auto res = this->removeItemFromTree(child->getName());

        if(res == false)
            return false;
    }

    return true;
}
