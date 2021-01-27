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

#include "MutuallyExclusiveListWidget.h"
#include "CheckableTreeModel.h"
#include "TreeItem.h"

#include <QList>
#include <QDebug>
#include <QPointer>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>

MutuallyExclusiveListWidget::MutuallyExclusiveListWidget(QWidget *parent, QString headerText) : QTreeView(parent)
{
    checkedItem = nullptr;

    treeModel = new CheckableTreeModel(this, headerText);
    this->setModel(treeModel);
    this->setWordWrap(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QWidget::customContextMenuRequested, this, &MutuallyExclusiveListWidget::showPopup);
}


TreeItem* MutuallyExclusiveListWidget::addItem(const QString item, TreeItem* parent)
{
    auto newItem = treeModel->addItemToTree(item, parent);

    connect(newItem, &TreeItem::removeThisItem, this, &MutuallyExclusiveListWidget::removeItem);

    connect(newItem, &TreeItem::itemChecked, this, &MutuallyExclusiveListWidget::handleItemChecked);
    connect(newItem, &TreeItem::itemUnchecked, this, &MutuallyExclusiveListWidget::handleItemUnchecked);

    return newItem;
}


void MutuallyExclusiveListWidget::removeItem(const QString& itemID)
{
    treeModel->removeItemFromTree(itemID);
}


void MutuallyExclusiveListWidget::handleItemChecked(const QString& itemID)
{
    auto item = treeModel->getTreeItem(itemID);

    if(item != checkedItem)
    {
        checkedItem = item;

        auto children = treeModel->getAllChildren();

        for(auto&& child : children)
        {
            if(child->getItemID().compare(itemID) != 0)
                child->setState(0);
        }
    }

    //        auto checkedItemName = checkedItem->getName();
    //        qDebug()<<"Checked Item: "<<checkedItemName;

    emit itemChecked(checkedItem);

}


void MutuallyExclusiveListWidget::handleItemUnchecked(const QString& itemID)
{
    auto item = treeModel->getTreeItem(itemID);

    if(item == checkedItem)
    {
        emit clearAll();
    }
}


void MutuallyExclusiveListWidget::clear(void)
{
    treeModel->clear();
}



void MutuallyExclusiveListWidget::showPopup(const QPoint &position)
{

    auto itemIndex = this->indexAt(position);

    auto itemName = itemIndex.data(0).toString();

    auto parentName = itemIndex.parent().data(0).toString();

    TreeItem *item = treeModel->getTreeItem(itemName,parentName);

    if (!item)
        return;

    QStringList popupList = item->getActionList();

    QMenu objectMenu;

    QList<QPointer<QAction> > actionList;
    if (!popupList.isEmpty())
    {
        actionList << objectMenu.addSeparator();

        for (int i = 0; i < popupList.count(); ++i)
        {
            if (popupList[i].toLower() == QString("Separator").toLower())
            {
                objectMenu.addSeparator();
            }
            else
            {
                QObject* parent = dynamic_cast<QObject*>(item);

                QAction *action = new QAction(popupList[i], parent);
                action->setObjectName(popupList[i]);
                actionList << action;
                connect(action, &QAction::triggered, this, &MutuallyExclusiveListWidget::runAction);
                objectMenu.addAction(action);
            }
        }
    }

    // Add the remove action at the end
    objectMenu.addSeparator();

    objectMenu.exec(this->mapToGlobal(position));

    disconnect(this, SLOT(runAction()));

    if (item)
    {
        for (int i = 0; i < actionList.count(); ++i)
        {
            objectMenu.removeAction(actionList[i]);
            delete actionList[i];
        }
    }

    return;
}


void MutuallyExclusiveListWidget::runAction()
{
    QObject *senderObject = sender();
    QString syntax = senderObject->objectName();
    syntax.remove('&');
    QStringList tempList = syntax.split(' ', QString::SkipEmptyParts);
    syntax.clear();

    for (int i = 0; i < tempList.count(); ++i)
    {
        syntax += tempList[i];
    }
    syntax.replace(0, 1, syntax.at(0).toLower());
    QByteArray byteStr= syntax.toLatin1();
    const char *charStr = byteStr.data();

    auto parent = senderObject->parent();
    bool ok = QMetaObject::invokeMethod(parent, charStr);

    if(!ok)
    {
        qCritical()<<"Something went wrong in "<<__FUNCTION__;
    }
}

