#ifndef ComponentDATABASE_H
#define ComponentDATABASE_H
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

#include <QMap>
#include <QVariant>

#include <qgsfeature.h>
#include <qgsattributes.h>
#include <qgsvectorlayer.h>

class PythonProgressDialog;

class QgsFeature;

class ComponentDatabase
{
public:
    ComponentDatabase();

    bool isEmpty(void);

    void addComponent(const int id, const QgsFeatureId fid);

    void clear(void);

    void startEditing(void);

    bool addFeatureToSelectedLayer(const int fid);

    bool removeFeaturesFromSelectedLayer(QgsFeatureIds& featureIds);
    bool clearSelectedLayer(void);

    bool updateComponentAttribute(const int ID, const QString& attribute, const QVariant& value);

    QVariant getAttributeValue(const int ID, const QString& attribute, const QVariant defaultVal = QVariant());

    void commitChanges(void);

    void setFields(const QgsFields &value);

    void setMainLayer(QgsVectorLayer *value);

    void setSelectedLayer(QgsVectorLayer *value);

    QgsFeature getFeature(const int id);

    QgsVectorLayer *getSelectedLayer() const;

    QgsVectorLayer *getMainLayer() const;

private:
    PythonProgressDialog* messageHandler;

    bool addFeatureToSelectedLayer(QgsFeature& feature, const int id);

    // Map to link R2D ids with QGIS feature ids
    QMap<int, QgsFeatureId> mapFeaturesMain;
    QMap<int, QgsFeatureId> mapFeaturesSelected;

    // Selected feature set
    QSet<QgsFeatureId> selectedFeaturesSet;

    // Set of layers that this component may have features in
    QgsVectorLayer* mainLayer = nullptr;
    QgsVectorLayer* selectedLayer = nullptr;

    QgsFields fields;
};

#endif // ComponentDATABASE_H
