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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "RendererTableView.h"
#include "RendererModel.h"

#include <QHeaderView>

RendererTableView::RendererTableView(QWidget* parent) : QTableView(parent)
{
    dataModel = new RendererModel(this);
    this->setModel(dataModel);

    this->setSelectionMode(QAbstractItemView::SingleSelection);

    this->verticalHeader()->hide();

    colorDelegate = std::make_unique<ColorDialogDelegate>();
    esriTypeComboDelegate = std::make_unique<RendererComboBoxItemDelegate>();

    this->setItemDelegateForColumn(6, colorDelegate.get());

    this->setItemDelegateForColumn(4, esriTypeComboDelegate.get());
    this->setItemDelegateForColumn(5, esriTypeComboDelegate.get());

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->horizontalHeader()->setSectionResizeMode(6,QHeaderView::ResizeToContents);
}


void RendererTableView::setRenderer(Esri::ArcGISRuntime::ClassBreaksRenderer* renderer)
{
    dataModel->setRenderer(renderer);
    esriTypeComboDelegate->setRenderer(renderer);

    int width = this->verticalHeader()->width();
    for(int column = 0; column < dataModel->columnCount(); ++column)
        width = width + this->columnWidth(column);

    int height = this->horizontalHeader()->height();
    for(int row = 0; row < dataModel->rowCount(); ++row)
        height += this->rowHeight(row);

    this->setMinimumWidth(600);
    this->setMinimumHeight(height);
}
