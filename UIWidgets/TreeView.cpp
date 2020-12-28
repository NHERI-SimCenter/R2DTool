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
#include "TreeModel.h"
#include "TreeView.h"
#include "TreeViewStyle.h"
#include "VisualizationWidget.h"

#include <QDebug>
#include <QMenu>
#include <QPointer>
#include <QTreeView>

TreeView::TreeView(QWidget *parent, VisualizationWidget* visWidget) : QTreeView(parent), theVisualizationWidget(visWidget)
{
    layersModel = new TreeModel(this);
    this->setModel(layersModel);

    this->setMaximumWidth(300);
    this->setWordWrap(true);

    this->setAcceptDrops(true);
    this->setDragEnabled(true);
    this->setDragDropOverwriteMode(false);
    this->setDefaultDropAction(Qt::MoveAction);
    this->setDragDropMode(QTreeView::InternalMove);
    this->setStyle(new TreeViewStyle(style()));
    this->setFocusPolicy(Qt::NoFocus);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(layersModel, &TreeModel::rowPositionChanged, visWidget, &VisualizationWidget::changeLayerOrder);

    // Connect the layers tree with the function that turns the layers visibility on/off in the GIS map
    connect(layersModel, &TreeModel::itemValueChanged, visWidget, &VisualizationWidget::handleLayerSelection);

    connect(this, &QWidget::customContextMenuRequested, this, &TreeView::showPopup);

}


TreeItem* TreeView::addItemToTree(const QString itemText, const QString layerID, TreeItem* parent)
{
    auto newLayer = layersModel->addItemToTree(itemText, layerID, parent);

    connect(newLayer, &TreeItem::opacityChanged, theVisualizationWidget, &VisualizationWidget::handleOpacityChange);

    return newLayer;
}


TreeItem* TreeView::getTreeItem(const QString& itemName, const QString& parentName) const
{
    return layersModel->getTreeItem(itemName, parentName);
}


void TreeView::showPopup(const QPoint &position)
{

    auto itemIndex = this->indexAt(position);

    auto itemName = itemIndex.data(0).toString();

    auto parentName = itemIndex.parent().data(0).toString();

    TreeItem *item = layersModel->getTreeItem(itemName,parentName);

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
                connect(action, &QAction::triggered, this, &TreeView::runAction);
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


void TreeView::runAction()
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


bool TreeView::removeItemFromTree(const QString& itemName)
{
    return layersModel->removeItemFromTree(itemName);
}


TreeModel *TreeView::getLayersModel() const
{
    return layersModel;
}
