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
#include "CheckableTreeModel.h"

#include <QDataStream>
#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QUuid>

CheckableTreeModel::CheckableTreeModel(QObject *parent, QString headerText) : QAbstractItemModel(parent)
{    
    rootItem = new TreeItem({headerText},0);
}


CheckableTreeModel::~CheckableTreeModel()
{
    delete rootItem;
}


int CheckableTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();

    return rootItem->columnCount();
}


QVariant CheckableTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if(item->getIsCheckable())
        {
            if(item->getState() == 0)
                return Qt::Unchecked;
            else if(item->getState() == 1)
                return Qt::PartiallyChecked;
            else if(item->getState() == 2)
                return Qt::Checked;
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}


Qt::ItemFlags CheckableTreeModel::flags(const QModelIndex &/*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;

    return flags;
}


QVariant CheckableTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex CheckableTreeModel::index(int row, int column, const QModelIndex &parent) const
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


QModelIndex CheckableTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->getParentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int CheckableTreeModel::rowCount(const QModelIndex &parent) const
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


TreeItem *CheckableTreeModel::getRootItem() const
{
    return rootItem;
}


bool CheckableTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    auto valueAsInt = value.toInt();

    std::function<void(TreeItem*, const int)> nestedItemChecker = [&](TreeItem* item, const int value)
    {
        auto children = item->getChildItems();

        for(auto&& it : children)
        {
            auto child = static_cast<TreeItem*>(it);
            nestedItemChecker(child,value);

            emit itemValueChanged(child);
        }

        item->setState(value);

        emit itemValueChanged(item);

        // Set the parent as a partial checked
        auto parent = static_cast<TreeItem*>(item->getParentItem());

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


TreeItem* CheckableTreeModel::addItemToTree(const QString itemText, TreeItem* parent)
{
    // Create a unique ID for the item as some items may have the same name
    auto id = QUuid::createUuid().toString();

    return this->addItemToTree(itemText,id,parent);
}


TreeItem* CheckableTreeModel::addItemToTree(const QString itemText, const QString itemID, TreeItem* parent)
{
    if(parent == nullptr)
        parent = rootItem;

    if(auto exists = this->getTreeItem(itemText, parent))
        return exists;

    QVector<QVariant> childText = {itemText};

    auto childItem = new TreeItem(childText, itemID, parent);

    parent->appendChild(childItem);

    emit layoutChanged();

    return childItem;
}


bool CheckableTreeModel::removeItemFromTree(const QString& itemID)
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


TreeItem* CheckableTreeModel::getTreeItem(const QString& itemName, const TreeItem* parent) const
{
    QString parentName;

    if(parent != nullptr)
    {
        parentName = parent->getName();
    }

    return this->getTreeItem(itemName,parentName);
}


TreeItem* CheckableTreeModel::getTreeItem(const QString& itemName, const QString& parentName) const
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

    return static_cast<TreeItem*>(nestedItemFinder(rootItem, itemName, parentName));
}


TreeItem* CheckableTreeModel::getTreeItem(const QString& itemID) const
{
    std::function<TreeItem* (TreeItem*, const QString&)> nestedItemFinder = [&](TreeItem* item, const QString& itemID) -> TreeItem*
    {
        // Check if this is the item
        auto thisItemID = item->getItemID();
        if(itemID.compare(thisItemID) == 0 )
            return item;

        // Now check the children of this item
        QVector<TreeItem *> children = item->getChildItems();
        for(int i = 0; i<children.size(); ++i)
        {
            auto child = children.at(i);

            // Check the childrens children
            if(auto foundChild = nestedItemFinder(child, itemID))
                return foundChild;
        }

        // Return a null item if none is found
        return nullptr;
    };

    return static_cast<TreeItem*>(nestedItemFinder(rootItem, itemID));
}


bool CheckableTreeModel::clear(void)
{
    auto children = rootItem->getChildItems();

    for(auto&& child : children)
    {
        auto res = this->removeItemFromTree(child->getItemID());

        if(res == false)
            return false;
    }

    rootItem->clear();

    return true;
}


QVector<TreeItem *> CheckableTreeModel::getAllChildren(void)
{
    QVector<TreeItem *> children{rootItem};

    std::function<void(TreeItem * item)> nestedChildFinder = [&](TreeItem * item){

        auto childItems = item->getChildItems();

        children.append(childItems);

        for(auto&& child : childItems)
            nestedChildFinder(child);

    };

    nestedChildFinder(rootItem);

    return children;
}
