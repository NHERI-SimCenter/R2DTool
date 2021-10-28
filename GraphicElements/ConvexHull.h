#ifndef ConvexHull_H
#define ConvexHull_H
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

#include <QMouseEvent>
#include <QObject>

class ArcGISVisualizationWidget;
class SimCenterMapGraphicsView;

namespace Esri
{
namespace ArcGISRuntime
{
class GraphicsOverlay;
class SimpleMarkerSymbol;
class Graphic;
class Point;
class SimpleLineSymbol;
class SimpleFillSymbol;
class MultipointBuilder;
class Geometry;
class Map;
}
}

class ConvexHull : public QObject
{
    Q_OBJECT

public:
    ConvexHull(ArcGISVisualizationWidget* visualizationWidget);

    void setupConvexHullObjects();

    bool getSelectingPoints() const;
    void setSelectingPoints(bool value);

signals:
    void selectionEnd();
    void selectionStart();

public slots:
    void plotConvexHull();
    void getConvexHullInputs();
    void resetConvexHull();
    void getItemsInConvexHull();
    void convexHullPointSelector(QMouseEvent& e);

private:

    // Create a graphic to display the convex hull selection
    Esri::ArcGISRuntime::GraphicsOverlay* m_graphicsOverlay = nullptr;
    Esri::ArcGISRuntime::SimpleMarkerSymbol* m_markerSymbol = nullptr;
    Esri::ArcGISRuntime::Graphic* m_inputsGraphic = nullptr;
    Esri::ArcGISRuntime::Graphic* m_convexHullGraphic = nullptr;
    Esri::ArcGISRuntime::SimpleLineSymbol* m_lineSymbol = nullptr;
    Esri::ArcGISRuntime::SimpleFillSymbol* m_fillSymbol = nullptr;
    Esri::ArcGISRuntime::MultipointBuilder* m_multipointBuilder = nullptr;
    bool selectingConvexHull;

    Esri::ArcGISRuntime::Map *mapGIS = nullptr;
    ArcGISVisualizationWidget* theVisualizationWidget = nullptr;
    SimCenterMapGraphicsView *mapViewWidget = nullptr;
};

#endif // ConvexHull_H
