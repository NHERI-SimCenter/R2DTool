#include "TreeView.h"
#include "VisualizationWidget.h"
#include "TreeModel.h"
#include "TreeViewStyle.h"
#include "TreeItem.h"

#include <QMenu>
#include <QPointer>
#include <QDebug>
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
