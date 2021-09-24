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

#include "GISSelectableComponent.h"
#include "Utils/PythonProgressDialog.h"
#include "QGISVisualizationWidget.h"

#include <qgsfield.h>
#include <qgsfields.h>
#include <qgsvectorlayer.h>

GISSelectableComponent::GISSelectableComponent(VisualizationWidget* visualizationWidget)
{
    messageHandler = PythonProgressDialog::getInstance();

    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visualizationWidget);
    assert(theVisualizationWidget);
}


GISSelectableComponent::~GISSelectableComponent()
{
    
}


bool GISSelectableComponent::addFeatureToSelectedLayer(QgsFeature& feature)
{
    auto id = feature.id();

    if(selectedFeaturesForAnalysis.contains(id))
        return true;

    auto res = selectedFeaturesLayer->dataProvider()->addFeature(feature, QgsFeatureSink::FastInsert);

    if(res == false)
    {
        messageHandler->appendErrorMessage("Error adding feature to selected feature layer");
        return false;
    }
    else
        selectedFeaturesForAnalysis.insert(id);

    return true;
}



bool GISSelectableComponent::removeFeaturesFromSelectedLayer(QgsFeatureIds& featureIds)
{
    auto res = selectedFeaturesLayer->dataProvider()->deleteFeatures(featureIds);

    return res;
}


bool GISSelectableComponent::clearSelectedLayer(void)
{
    auto res = selectedFeaturesLayer->dataProvider()->truncate();

    return res;
}


void GISSelectableComponent::clearLayerSelectedForAnalysis(void)
{
    if(selectedFeaturesForAnalysis.empty())
        return;

    this->clearSelectedLayer();

    selectedFeaturesForAnalysis.clear();
}


void GISSelectableComponent::clear(void)
{
    if(selectedFeaturesLayer != nullptr)
    {
        theVisualizationWidget->removeLayer(selectedFeaturesLayer);

        delete selectedFeaturesLayer;

        selectedFeaturesLayer = nullptr;
    }

    selectedFeaturesForAnalysis.clear();
}


bool GISSelectableComponent::updateSelectedComponentAttribute(QgsFeatureId id, int field, const QVariant& value)
{

    if(selectedFeaturesForAnalysis.empty())
    {
        messageHandler->appendErrorMessage("Selected features map is empty, nothing to update");
        return false;
    }

    auto res = selectedFeaturesLayer->changeAttributeValue(id, field,value);

    if(res == false)
    {
        messageHandler->appendErrorMessage("Failed to update feature "+QString::number(id));
        return false;
    }

    return true;
}


