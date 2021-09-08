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

#ifdef ARC_GIS
#include <Feature.h>
#include <FeatureTable.h>

namespace Esri
{
namespace ArcGISRuntime
{
class Feature;
}
}
#endif


#ifdef Q_GIS
#include <qgsfeature.h>
#include <qgsattributes.h>

class QgsFeature;
#endif

struct Component
{
public:

    void addResult(const QString& key, const double res)
    {
        ResultsValues.insert(key,res);
    }

    QVariant getAttributeValue(const QString& key)
    {
        return ComponentAttributes.value(key);
    }

    double getResultValue(const QString& key)
    {
        return ResultsValues.value(key);
    }

    int setAttributeValue(const QString& attribute, const QVariant& value)
    {
        ComponentAttributes[attribute] = value;

        if(ComponentFeature != nullptr)
        {            
            auto res = ComponentFeature->setAttribute(attribute,value);

            if(res == false)
            {
                qDebug()<<"Failed to update feature "<<attribute<<" in component "<<ID;
                return -1;
            }
        }

        return 0;
    }

    bool isValid(void)
    {
        if(ComponentFeature == nullptr || ComponentAttributes.empty())
            return false;

        return true;
    }

    int ID = -1;

    // Unique id of this component
    QString UID = "NULL";

#ifdef ARC_GIS
    // The Component feature in the GIS widget
    Esri::ArcGISRuntime::Feature* ComponentFeature = nullptr;
#endif

#ifdef Q_GIS
    QgsFeature* ComponentFeature = nullptr;
#endif

    // Map to store the Component attributes
    QMap<QString, QVariant> ComponentAttributes;

    // Map to store the results - the QString (key) is the header text while the double is the value for that Component
    QMap<QString, double> ResultsValues;
};


class ComponentDatabase
{
public:
    ComponentDatabase();

    // Gets the Component as a modifiable reference
    Component& getComponent(const int ID);

    Component getComponent(const QString UID);

    int getNumberOfComponents();

    void addComponent(int ID, Component& asset);

    void clear(void);

    QMap<int, Component> getComponentsMap() const;

    void updateComponentAttribute(const int ID, const QString& attribute, const QVariant& value);

private:

    QMap<int,Component> ComponentMap;

};

#endif // ComponentDATABASE_H
