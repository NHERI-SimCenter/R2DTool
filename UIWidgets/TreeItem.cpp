/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Written by: Stevan Gavrilovic
// Latest revision: 10.12.2020

#include "TreeItem.h"

#include <QDebug>
#include <QDialog>
#include <QSlider>
#include <QGridLayout>
#include <QLabel>

TreeItem::TreeItem(const QVector<QVariant> &data, const QString& ID, TreeItem *parent) : QObject(), itemData(data), parentItem(parent), itemID(ID)
{
    opacityDialog = nullptr;
    itemName = data.at(0).toString();
    currentState = 2;
}


TreeItem::~TreeItem()
{
    qDeleteAll(vecChildItems);

    if (opacityDialog)
        delete opacityDialog;
}


QStringList TreeItem::getActionList()
{
    QStringList actionList;

    actionList << "&Change Opacity"
               << "Separator";

    return actionList;
}


void TreeItem::moveChild(int sourceRow, int destinationRow)
{

    if(vecChildItems.empty())
        return;

    if(sourceRow < 0)
        sourceRow = 0;

    if(destinationRow >= vecChildItems.size())
        destinationRow = vecChildItems.size() -1;

    vecChildItems.move(sourceRow, destinationRow);

}


TreeItem *TreeItem::findChild(QString name)
{

    // First check if this is the item we are looking for
    if(name.compare(this->data(0).toString()) == 0)
        return this;

    // Now check the children of this item
    for(auto&& it : vecChildItems)
    {
        if(name.compare(it->data(0).toString()) == 0)
            return it;

        // Check the childrens children
        if(auto found = it->findChild(name))
            return found;

    }

    // Finally return a null pointer if no children are found
    return nullptr;
}


void TreeItem::setState(int set)
{
    if(set == 1 && !vecChildItems.empty())
    {
        int state = vecChildItems.at(0)->getState();

        for(auto&& it:vecChildItems)
        {
            // If any children are unchecked or partially set do nothing
            if(it->getState() != state)
            {
                currentState = set;

                if(parentItem)
                    parentItem->setState(1);

                return;
            }
        }

        // If all children are checked or unchecked, set the state to this item as well
        currentState = state;

        if(parentItem)
            parentItem->setState(1);

        return;
    }

    currentState = set;
}


QVector<TreeItem *> TreeItem::getChildItems() const
{
    return vecChildItems;
}


int TreeItem::getState() const
{
    return currentState;
}


void TreeItem::appendChild(TreeItem *child)
{
    vecChildItems.append(child);
}


void TreeItem::removeChild(TreeItem *child)
{
    int index = -1;

    for(int i = 0; i<vecChildItems.size(); ++i)
    {
        if(vecChildItems.at(i) == child)
        {
            index = i;
            break;
        }
    }

    if(index != -1)
        vecChildItems.remove(index);
}


void TreeItem::removeChild(int row)
{
    auto numChildren = vecChildItems.size();

    if(row<0 || row>numChildren-1)
        return;

    vecChildItems.remove(row);
}


TreeItem *TreeItem::child(int row)
{
    if (row < 0 || row >= vecChildItems.size())
        return nullptr;

    return vecChildItems.at(row);
}


int TreeItem::childCount() const
{
    return vecChildItems.count();
}


int TreeItem::columnCount() const
{
    return itemData.count();
}


QVariant TreeItem::data(int column) const
{
    if (column < 0 || column >= itemData.size())
        return QVariant();

    return itemData.at(column);
}


TreeItem* TreeItem::getParentItem()
{
    return parentItem;
}


int TreeItem::row() const
{
    if (parentItem)
        return parentItem->vecChildItems.indexOf(const_cast<TreeItem*>(this));

    return -1;
}


void TreeItem::changeOpacity()
{
    if (!opacityDialog)
    {
        opacityDialog = new QDialog();

        auto slider = new QSlider(opacityDialog);

        connect(slider, &QAbstractSlider::sliderMoved, this, &TreeItem::handleChangeOpacity);

        slider->setOrientation(Qt::Horizontal);

        slider->setValue(100);

        slider->setTickPosition(QSlider::TickPosition::TicksAbove);

        auto dialogLayout = new QGridLayout(opacityDialog);

        auto label = new QLabel("0.0",opacityDialog);
        auto label2  = new QLabel("1.0",opacityDialog);

        dialogLayout->addWidget(label, 0, 0);
        dialogLayout->addWidget(slider, 0, 1);
        dialogLayout->addWidget(label2, 0, 2);

        opacityDialog->setLayout(dialogLayout);
        opacityDialog->setWindowTitle("Adjust opacity of layer "+itemName);

        opacityDialog->setMinimumWidth(400);
        opacityDialog->setMinimumHeight(150);

    }

    opacityDialog->show();
    opacityDialog->raise();
    opacityDialog->activateWindow();
}


void TreeItem::handleChangeOpacity(int value)
{
    if(itemID.isEmpty())
        return;

    auto opacity = static_cast<double>(value+1)/100.0;

    emit opacityChanged(itemID, opacity);
}

QString TreeItem::getItemID() const
{
    return itemID;
}

QString TreeItem::getName() const
{
    return itemName;
}
