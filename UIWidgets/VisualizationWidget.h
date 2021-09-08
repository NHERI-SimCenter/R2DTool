#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SimCenterAppWidget.h"
#include "GISLegendView.h"

#include <QMap>
#include <QObject>
#include <QUuid>

class SimCenterMapGraphicsView;
class ComponentInputWidget;

class QVBoxLayout;

class VisualizationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit VisualizationWidget(QWidget* parent);
    virtual ~VisualizationWidget();

    SimCenterMapGraphicsView* getMapViewWidget() const;

    // Note: the component type must match the "AssetType" value set to the features
    void registerComponentWidget(const QString assetType, ComponentInputWidget* widget);

    ComponentInputWidget* getComponentWidget(const QString type);


    // Get the visualization widget
    virtual QWidget *getVisWidget() = 0;

    // Get the latitude or longitude from a point on the screen
    virtual double getLatFromScreenPoint(const QPointF& point) = 0;
    virtual double getLongFromScreenPoint(const QPointF& point) = 0;

    virtual QPointF getScreenPointFromLatLong(const double& latitude, const double& longitude) = 0;

    QString createUniqueID(void);

    virtual void takeScreenShot(void);

    virtual void clear(void) = 0;

    // Clear the currently selected features (i.e., features highlighted by clicking on them)
    virtual void clearSelection(void) = 0;

    // Updates the value of an attribute for a selected component
    void updateSelectedComponent(const QString& assetType, const QString& uid, const QString& attribute, const QVariant& value);

    void setLegendView(GISLegendView* legndView);
    GISLegendView *getLegendView() const;

    // Hides the map legend
    void hideLegend(void);

signals:
    // Emit a screen shot of the current GIS view
    void emitScreenshot(QImage img);

public slots:    

    virtual void setCurrentlyViewable(bool status) = 0;
    virtual void handleLegendChange(const QString layerUID) = 0;

private slots:

protected:

    // Map to hold the component input widgets (key = type of component or asset, e.g., BUILDINGS)
    QMap<QString, ComponentInputWidget*> componentWidgetsMap;

    SimCenterMapGraphicsView *mapViewWidget = nullptr;

    // The legend view
    GISLegendView* legendView;

    QVBoxLayout *mainLayout;

};

#endif // VISUALIZATIONWIDGET_H
