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

#include "ComponentDatabase.h"
#include "Utils/PythonProgressDialog.h"

#include <qgsfeature.h>

ComponentDatabase::ComponentDatabase()
{
    messageHandler = PythonProgressDialog::getInstance();
}


bool ComponentDatabase::isEmpty(void)
{
    if(mainLayer == nullptr)
        return true;

    if(mainLayer->featureCount() == 0)
        return true;

    return false;
}


void ComponentDatabase::clear(void)
{
    mainLayer = nullptr;
    mapFeaturesMain.clear();
    mapFeaturesSelected.clear();
    fields.clear();
    selectedFeaturesSet.clear();

    //    if(selectedLayer != nullptr)
    //    {
    //        theVisualizationWidget->removeLayer(selectedLayer);

    //        delete selectedLayer;

    selectedLayer = nullptr;
    //    }
}


QgsFeature ComponentDatabase::getFeature(const int id)
{
    auto fid = mapFeaturesMain.value(id, QgsFeatureId());

    if(FID_IS_NULL(fid))
        return QgsFeature();

    return mainLayer->getFeature(fid);
}

QgsVectorLayer *ComponentDatabase::getSelectedLayer() const
{
    return selectedLayer;
}

QgsVectorLayer *ComponentDatabase::getMainLayer() const
{
    return mainLayer;
}


void ComponentDatabase::startEditing(void)
{
    selectedLayer->startEditing();
    mainLayer->startEditing();
}


void ComponentDatabase::commitChanges(void)
{

    //    selectedLayer->setAllowCommit(true);
    selectedLayer->commitChanges(true);

    //      selectedLayer->endEditCommand();

    //      selectedLayer->destroyEditCommand();
    //    selectedLayer->triggerRepaint();

    //    selectedLayer->dataProvider()->updateExtents();
    selectedLayer->updateExtents();

    //    auto err = selectedLayer->commitErrors();

}

void ComponentDatabase::setFields(const QgsFields &value)
{
    fields = value;
}

void ComponentDatabase::setMainLayer(QgsVectorLayer *value)
{
    mainLayer = value;
}

void ComponentDatabase::setSelectedLayer(QgsVectorLayer *value)
{
    selectedLayer = value;
}


void ComponentDatabase::addComponent(const int id, const QgsFeatureId fid)
{
    mapFeaturesMain.insert(id,fid);
}


bool ComponentDatabase::addFeatureToSelectedLayer(const int id)
{
    auto fid = mapFeaturesMain.value(id, std::numeric_limits<QgsFeatureId>::min());

    if(selectedFeaturesSet.contains(fid))
        return true;

    auto feature = this->getFeature(fid);
    if(feature.isValid() == false)
    {
        messageHandler->appendErrorMessage("Error getting the feature from the database");
        return false;
    }

    return this->addFeatureToSelectedLayer(feature, id);
}


bool ComponentDatabase::addFeatureToSelectedLayer(QgsFeature& feature, const int id)
{

    auto fid = feature.id();

    auto res = selectedLayer->dataProvider()->addFeature(feature);

    // auto res = selectedLayer->addFeature(feature/*, QgsFeatureSink::FastInsert*/);

    if(res == false)
    {
        messageHandler->appendErrorMessage("Error adding feature to selected feature layer");
        return false;
    }

    // Get the updated id from the feature
    auto newId = feature.id();

    selectedFeaturesSet.insert(fid);

    mapFeaturesSelected.insert(id,newId);

    return true;
}



bool ComponentDatabase::removeFeaturesFromSelectedLayer(QgsFeatureIds& featureIds)
{
    auto res = selectedLayer->dataProvider()->deleteFeatures(featureIds);

    return res;
}


bool ComponentDatabase::clearSelectedLayer(void)
{
    auto res = selectedLayer->dataProvider()->truncate();

    return res;
}



bool ComponentDatabase::updateComponentAttribute(const int id, const QString& attribute, const QVariant& value)
{
    auto fid = mapFeaturesMain.value(id, std::numeric_limits<QgsFeatureId>::min());

    auto field = fields.indexFromName(attribute);

    if(FID_IS_NULL(fid) || field == -1)
        return false;

    auto res = mainLayer->changeAttributeValue(fid,field,value);

    if(!res)
        return res;

    // Update the selected layer if there is one...
    if(selectedLayer != nullptr)
    {
        auto fidSel = mapFeaturesSelected.value(id, std::numeric_limits<QgsFeatureId>::min());

        if(FID_IS_NULL(fidSel))
            return false;

        auto res2 = selectedLayer->changeAttributeValue(fidSel,field,value);

        if(!res2)
            return res2;
    }

    return true;
}


QVariant ComponentDatabase::getAttributeValue(const int id, const QString& attribute, const QVariant defaultVal)
{
    QVariant val(defaultVal);
    auto fid = mapFeaturesMain.value(id, std::numeric_limits<QgsFeatureId>::min());

    if(FID_IS_NULL(fid))
        return val;

    auto feature = mainLayer->getFeature(fid);

    if(feature.isValid())
        val = feature.attribute(attribute);

    if(!val.isValid())
        return defaultVal;

    return val;
}
