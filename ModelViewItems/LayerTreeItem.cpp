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

#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>

LayerTreeItem::LayerTreeItem(const QVector<QVariant> &data, const QString& ID, TreeItem *parent) : TreeItem(data,ID,parent)
{
    opacityDialog = nullptr;
    currentState = 2;
}


LayerTreeItem::~LayerTreeItem()
{
    if (opacityDialog)
        delete opacityDialog;
}


QStringList LayerTreeItem::getActionList()
{
    QStringList actionList = TreeItem::getActionList();

    actionList << "&Change Opacity"
               << "&Manage Layer"
               << "&Zoom to Layer"
               << "Separator";

    return actionList;
}


void LayerTreeItem::changeOpacity()
{
    if (!opacityDialog)
    {
        opacityDialog = new QDialog();

        auto slider = new QSlider(opacityDialog);

        connect(slider, &QAbstractSlider::sliderMoved, this, &LayerTreeItem::handleChangeOpacity);

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


void LayerTreeItem::zoomtoLayer()
{
    if(itemID.isEmpty())
        return;

    emit zoomLayerExtents(itemID);

}


void LayerTreeItem::handleChangeOpacity(int value)
{
    if(itemID.isEmpty())
        return;

    auto opacity = static_cast<double>(value+1)/100.0;

    emit opacityChanged(itemID, opacity);
}


void LayerTreeItem::manageLayer()
{
    emit plotColorChanged(itemID);
}

