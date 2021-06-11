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

