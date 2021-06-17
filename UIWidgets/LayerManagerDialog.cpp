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

#include "LayerManagerDialog.h"
#include "RendererTableView.h"
#include "LayerManagerTableView.h"

#include "FeatureCollectionLayer.h"
#include "Renderer.h"
#include "SimpleRenderer.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "ClassBreaksRenderer.h"

#include <QColorDialog>
#include <QVBoxLayout>
#include <QPushButton>

using namespace Esri::ArcGISRuntime;

LayerManagerDialog::LayerManagerDialog(QWidget* parent) : QObject(parent)
{
    // Layer manager
    layerManagerDialog = new QDialog(parent);
    layerManagerDialog->setWindowTitle("Manage Layer");
    layerManagerDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    layerManagerDialog->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto lmLayout = new QVBoxLayout(layerManagerDialog);

    layerManagerTableView = new LayerManagerTableView(layerManagerDialog);

    auto lmDoneButton = new QPushButton("Done",layerManagerDialog);
    connect(lmDoneButton,&QPushButton::pressed,layerManagerDialog,&QDialog::close);

    auto lmButtonsLayout = new QHBoxLayout();
    lmButtonsLayout->addWidget(lmDoneButton, Qt::AlignCenter);

    lmLayout->addWidget(layerManagerTableView);
    lmLayout->addLayout(lmButtonsLayout);


    // Class breaks manager
    classBreaksDialog = new QDialog(parent);
    classBreaksDialog->setWindowTitle("Manage Layer");
    classBreaksDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    classBreaksDialog->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto cbLayout = new QVBoxLayout(classBreaksDialog);

    rendererTableView = new RendererTableView(classBreaksDialog);

    auto cbDoneButton = new QPushButton("Done",classBreaksDialog);
    connect(cbDoneButton,&QPushButton::pressed,classBreaksDialog,&QDialog::close);

    auto cbButtonsLayout = new QHBoxLayout();
    cbButtonsLayout->addWidget(cbDoneButton,Qt::AlignCenter);

    cbLayout->addWidget(rendererTableView);
    cbLayout->addLayout(cbButtonsLayout);
}


int LayerManagerDialog::changeClassBreaksRenderer(Esri::ArcGISRuntime::ClassBreaksRenderer* cbRenderer)
{
    rendererTableView->setRenderer(cbRenderer);

    return classBreaksDialog->exec();
}


int LayerManagerDialog::changeLayer(Esri::ArcGISRuntime::FeatureCollectionLayer* layer)
{



    auto featCollection = layer->featureCollection();

    auto tables = featCollection->tables();

    for(int i = 0;i<tables->size(); ++i)
    {
        auto table = tables->at(i);

        // Get the renderer
        Renderer* tableRenderer = table->renderer();

        if(auto simpleRenderer = dynamic_cast<SimpleRenderer*>(tableRenderer))
        {
            layerManagerTableView->setLayer(layer);

            return layerManagerDialog->exec();
        }
        else if(auto cbRenderer = dynamic_cast<ClassBreaksRenderer*>(tableRenderer))
        {
            if(cbRenderer->classBreaks()->size() == 0)
                continue;

            rendererTableView->setRenderer(cbRenderer);

            return classBreaksDialog->exec();

        }
    }


    return 0;
}

