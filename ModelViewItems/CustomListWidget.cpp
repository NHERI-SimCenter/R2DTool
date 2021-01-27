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

#include "CustomListWidget.h"
#include "ListTreeModel.h"
#include "TreeItem.h"

#include <QList>
#include <QDebug>
#include <QPointer>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>

CustomListWidget::CustomListWidget(QWidget *parent, QString headerText) : QTreeView(parent)
{
    treeModel = new ListTreeModel(headerText, this);
    this->setModel(treeModel);
    this->setWordWrap(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QWidget::customContextMenuRequested, this, &CustomListWidget::showPopup);
}


TreeItem* CustomListWidget::addItem(const QString item, QString model, const double weight, TreeItem* parent)
{
    auto num = treeModel->rowCount();

    QString newItemText = item + " - weight="+ QString::number(weight);

    auto newItem = treeModel->addItemToTree(newItemText, parent);

    newItem->setProperty("Model",model);
    newItem->setProperty("Weight",weight);

    connect(newItem, &TreeItem::removeThisItem, this, &CustomListWidget::removeItem);

    this->update();

    return newItem;
}


TreeItem* CustomListWidget::addItem(const QString item, TreeItem* parent)
{
    auto num = treeModel->rowCount();

    QString newItemText = QString::number(num+1) + ". " + item;

    auto newItem = treeModel->addItemToTree(newItemText, parent);


    return newItem;
}


void CustomListWidget::removeItem(const QString& itemID)
{
    treeModel->removeItemFromTree(itemID);

    this->update();
}


void CustomListWidget::clear(void)
{
    treeModel->clear();
}


QVariantList CustomListWidget::getListOfWeights(TreeItem* parentItem) const
{
    QVector<TreeItem *> childVec;

    if(parentItem != nullptr)
        childVec = parentItem->getChildItems();
    else
        childVec = treeModel->getAllChildren();

    QVariantList ListOfWeights;

    for(auto&& it : childVec)
    {
       auto weightObj = it->property("Weight");

       if(!weightObj.isValid())
           continue;

       ListOfWeights << weightObj;
    }

    return ListOfWeights;
}


QVariantList CustomListWidget::getListOfModels(TreeItem* parentItem) const
{
    QVector<TreeItem *> childVec;

    if(parentItem != nullptr)
        childVec = parentItem->getChildItems();
    else
        childVec = treeModel->getAllChildren();

    QVariantList ListOfModels;

    for(auto&& it : childVec)
    {
       auto modelObj = it->property("Model");

       if(!modelObj.isValid())
           continue;

       ListOfModels << modelObj;
    }

    return ListOfModels;
}


int CustomListWidget::getNumberOfItems()
{
    return treeModel->getNumberOfItems();
}


void CustomListWidget::showPopup(const QPoint &position)
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
                connect(action, &QAction::triggered, this, &CustomListWidget::runAction);
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


void CustomListWidget::update()
{
    auto childVec = treeModel->getAllChildren();

    std::function<void(TreeItem* item)> nestedUpdater = [](TreeItem* item){

        auto childItems = item->getChildItems();

        for(auto&& it : childItems)
        {
           auto modelObj = it->property("Model");

           if(!modelObj.isValid())
               continue;

           auto modelStr = modelObj.toString();

           auto rowNum = it->row();

           auto weightObj = it->property("Weight");

           auto weightStr = weightObj.toString();

           QString newItemText = QString::number(rowNum+1) + ". " + modelStr + " - weight="+ weightStr;

           it->setData(newItemText,0);
        }
    };

    for(auto&& it : childVec)
    {
        nestedUpdater(it);
    }

}


void CustomListWidget::runAction()
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

