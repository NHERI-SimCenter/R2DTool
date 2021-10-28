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
#include "LayerTreeView.h"
#include "TreeViewStyle.h"
#include "ArcGISVisualizationWidget.h"

#include <QDebug>
#include <QMenu>
#include <QPointer>

LayerTreeView::LayerTreeView(QWidget *parent, ArcGISVisualizationWidget* visWidget) : QTreeView(parent), theVisualizationWidget(visWidget)
{
    layersModel = new LayerTreeModel(this);
    this->setModel(layersModel);

    this->setMaximumWidth(300);
    this->setWordWrap(true);
    resizeColumnToContents(0);

    this->setDefaultDropAction(Qt::MoveAction);
    this->setDragDropOverwriteMode(false);
    this->setFocusPolicy(Qt::NoFocus);
    this->setStyle(new TreeViewStyle(style()));
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setDragEnabled(true);
    this->viewport()->setAcceptDrops(true);
    this->setDragDropMode(QAbstractItemView::InternalMove);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(layersModel, &LayerTreeModel::rowPositionChanged, visWidget, &ArcGISVisualizationWidget::changeLayerOrder);

    // Connect the layers tree with the function that turns the layers visibility on/off in the GIS map
    connect(layersModel, &LayerTreeModel::itemValueChanged, visWidget, &ArcGISVisualizationWidget::handleLayerChecked);

    connect(this, &QWidget::customContextMenuRequested, this, &LayerTreeView::showPopup);

    connect(this, &QTreeView::clicked, this, &LayerTreeView::itemSelected);

}


LayerTreeItem* LayerTreeView::addItemToTree(const QString itemText, const QString layerID, LayerTreeItem* parent)
{
    auto newItem = layersModel->addItemToTree(itemText, layerID, parent);

    connect(newItem, &LayerTreeItem::opacityChanged, theVisualizationWidget, &ArcGISVisualizationWidget::handleOpacityChange);
    connect(newItem, &LayerTreeItem::plotColorChanged, theVisualizationWidget, &ArcGISVisualizationWidget::handlePlotColorChange);
    connect(newItem, &LayerTreeItem::zoomLayerExtents, theVisualizationWidget, &ArcGISVisualizationWidget::zoomToLayer);
    connect(newItem, &TreeItem::removeThisItem, this, &LayerTreeView::removeItemFromTree);
    connect(newItem, &TreeItem::removingChildItem, this, &LayerTreeView::removeLayer);

    return newItem;
}


LayerTreeItem* LayerTreeView::getTreeItem(const QString& itemName, const QString& parentName) const
{
    return layersModel->getLayerTreeItem(itemName, parentName);
}


LayerTreeItem* LayerTreeView::getTreeItem(const QString& itemID) const
{
    return layersModel->getLayerTreeItem(itemID);
}


void LayerTreeView::showPopup(const QPoint &position)
{

    auto itemIndex = this->indexAt(position);

    auto itemName = itemIndex.data(0).toString();

    auto parentName = itemIndex.parent().data(0).toString();

    LayerTreeItem *item = layersModel->getLayerTreeItem(itemName,parentName);

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
                connect(action, &QAction::triggered, this, &LayerTreeView::runAction);
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


void LayerTreeView::runAction()
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


void LayerTreeView::removeLayer(const QString& layerID)
{
    if(!layerID.isEmpty())
        theVisualizationWidget->removeLayerFromMap(layerID);
}


bool LayerTreeView::removeItemFromTree(const QString& itemID)
{
//    this->removeLayer(itemID);

    return layersModel->removeItemFromTree(itemID);
}


LayerTreeModel *LayerTreeView::getLayersModel() const
{
    return layersModel;
}


void LayerTreeView::clear(void)
{
    layersModel->clear();
}


void LayerTreeView::itemSelected(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    auto selectedLayerUID = layersModel->uidItem(index);

    theVisualizationWidget->handleLegendChange(selectedLayerUID);
}


void LayerTreeView::selectRow(int i)
{
    auto rowIndex = layersModel->index(i);
    this->setCurrentIndex(rowIndex);
}


int LayerTreeView::setCurrentItem(const QString& itemID)
{
    auto currItem = layersModel->getLayerTreeItem(itemID);

    if(currItem == nullptr)
        return -1;

    auto row = currItem->row();

    this->selectRow(row);
}

