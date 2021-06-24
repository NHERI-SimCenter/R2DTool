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

#include "LayerComboBoxItemDelegate.h"

#include "FeatureCollectionLayer.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "SimpleFillSymbol.h"

#include <QComboBox>
#include <QSpinBox>

using namespace Esri::ArcGISRuntime;

LayerComboBoxItemDelegate::LayerComboBoxItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}


QWidget *LayerComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{

    if (!index.isValid())
        return nullptr;

    auto col = index.column();
    auto row = index.row();

    if(col == 1)
    {
        auto renderer = m_layer->featureCollection()->tables()->at(row)->renderer();
        auto rendererType = renderer->rendererType();

        if(rendererType == RendererType::SimpleRenderer)
        {
            auto s_renderer = static_cast<SimpleRenderer*>(renderer);

            auto symbol = s_renderer->symbol();

            auto s_symbolType = symbol->symbolType();

            if(s_symbolType == SymbolType::SimpleMarkerSymbol)
            {
                auto s_markerSymbol = static_cast<SimpleMarkerSymbol*>(symbol);

                auto s_markerSymbolStyle = s_markerSymbol->style();

                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
                editor->setVisible(true);

                // Circle = 0,
                // Cross = 1,
                // Diamond = 2,
                // Square = 3,
                // Triangle = 4,
                // X = 5

                editor->addItem("Circle");
                editor->addItem("Cross");
                editor->addItem("Diamond");
                editor->addItem("Square");
                editor->addItem("Triangle");
                editor->addItem("X");

                auto currentIndex = static_cast<int>(s_markerSymbolStyle);
                editor->setCurrentIndex(currentIndex);

                return editor;
            }
            else if(s_symbolType == SymbolType::SimpleLineSymbol)
            {
                auto s_markerSymbol = static_cast<SimpleLineSymbol*>(symbol);

                auto s_markerSymbolStyle = s_markerSymbol->style();

                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
                editor->setVisible(true);

                //   Dash = 0,
                //   DashDot = 1,
                //   DashDotDot = 2,
                //   Dot = 3,
                //   Null = 4,
                //   Solid = 5

                editor->addItem("Dash");
                editor->addItem("Dash Dot");
                editor->addItem("Dash Dot Dot");
                editor->addItem("Dot");
                editor->addItem("Null");
                editor->addItem("Solid");

                auto currentIndex = static_cast<int>(s_markerSymbolStyle);
                editor->setCurrentIndex(currentIndex);

                return editor;
            }
            else if(s_symbolType == SymbolType::SimpleFillSymbol)
            {
                auto s_markerSymbol = static_cast<SimpleFillSymbol*>(symbol);

                auto s_markerSymbolStyle = s_markerSymbol->style();

                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
                editor->setVisible(true);

                // BackwardDiagonal = 0,
                // Cross = 1,
                // DiagonalCross = 2,
                // ForwardDiagonal = 3,
                // Horizontal = 4,
                // Null = 5,
                // Solid = 6,
                // Vertical = 7

                editor->addItem("Backward Diagonal");
                editor->addItem("Cross");
                editor->addItem("Diagonal Cross");
                editor->addItem("Forward Diagonal");
                editor->addItem("Horizontal");
                editor->addItem("Null");
                editor->addItem("Solid");
                editor->addItem("Vertical");

                auto currentIndex = static_cast<int>(s_markerSymbolStyle);
                editor->setCurrentIndex(currentIndex);

                return editor;
            }
            else
            {
                return nullptr;
            }
        }

    }
    else if(col == 2)
    {
        QString isValid = index.data(Qt::DisplayRole).toString();

        if(isValid.compare("N/A") == 0)
            return nullptr;

        QSpinBox *editor = new QSpinBox(parent);

        editor->setFrame(false);
        editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        editor->setVisible(true);
        editor->setMinimum(0);
        editor->setMaximum(100);

        int currVal = index.data(Qt::DisplayRole).toInt();

        editor->setValue(currVal);

        return editor;
    }

    return nullptr;
}


void LayerComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    auto col = index.column();

    if(col == 1)
    {
        QString value = index.model()->data(index, Qt::DisplayRole).toString();

        QComboBox *comboBox = static_cast<QComboBox*>(editor);

        auto currentIndex = comboBox->findText(value);
        comboBox->setCurrentIndex(currentIndex);
    }
    else if(col == 2)
    {
        int value = index.model()->data(index, Qt::DisplayRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);

        spinBox->setValue(value);
    }
}


void LayerComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

    auto col = index.column();
    if(col == 1)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);

        QString value = comboBox->currentText();

        model->setData(index, value, Qt::EditRole);
    }
    else if(col == 2)
    {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);

        int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }
}


void LayerComboBoxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void LayerComboBoxItemDelegate::setLayer(Esri::ArcGISRuntime::FeatureCollectionLayer *layer)
{
    m_layer = layer;
}
