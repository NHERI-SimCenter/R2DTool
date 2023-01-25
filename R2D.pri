>#*****************************************************************************
# Copyright (c) 2016-2021, The Regents of the University of California (Regents).
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies,
# either expressed or implied, of the FreeBSD Project.
#
# REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
# THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
# PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
# UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#
#***************************************************************************

# Written by: Stevan Gavrilovic



INCLUDEPATH += $$PWD \
               $$PWD/Utils \
               $$PWD/styles \
               $$PWD/UIWidgets \
               $$PWD/ModelViewItems \
               $$PWD/GraphicElements \
               $$PWD/Events/UI \
               $$PWD/Tools \


SOURCES +=  $$PWD/Events/UI/EarthquakeRuptureForecast.cpp \
            $$PWD/Events/UI/BedrockDepth.cpp \
            $$PWD/Events/UI/BedrockDepthWidget.cpp \
            $$PWD/Events/UI/EarthquakeRuptureForecastWidget.cpp \
            $$PWD/Events/UI/EventGMDirWidget.cpp \
            $$PWD/Events/UI/GMPE.cpp \
            $$PWD/Events/UI/GMPEWidget.cpp \
            $$PWD/Events/UI/GMWidget.cpp \
            $$PWD/Events/UI/GmAppConfig.cpp \
            $$PWD/Events/UI/GmAppConfigWidget.cpp \
            $$PWD/Events/UI/GmCommon.cpp \
            $$PWD/Events/UI/GridDivision.cpp \
            $$PWD/Events/UI/HBoxFormLayout.cpp \
            $$PWD/Events/UI/HazardOccurrence.cpp \
            $$PWD/Events/UI/HazardOccurrenceWidget.cpp \
            $$PWD/Events/UI/IntensityMeasure.cpp \
            $$PWD/Events/UI/IntensityMeasureWidget.cpp \
            $$PWD/Events/UI/Location.cpp \
            $$PWD/Events/UI/OpenQuakeClassical.cpp \
            $$PWD/Events/UI/OpenQuakeClassicalWidget.cpp \
            $$PWD/Events/UI/OpenQuakeScenario.cpp \
            $$PWD/Events/UI/OpenQuakeScenarioWidget.cpp \
            $$PWD/Events/UI/OpenQuakeUserSpecifiedWidget.cpp \
            $$PWD/Events/UI/PeerLoginDialog.cpp \
            $$PWD/Events/UI/PeerNGAWest2Client.cpp \
            $$PWD/Events/UI/PointSourceRupture.cpp \
            $$PWD/Events/UI/PointSourceRuptureWidget.cpp \
            $$PWD/Events/UI/QGISSiteInputWidget.cpp \
            $$PWD/Events/UI/RecordSelectionConfig.cpp \
            $$PWD/Events/UI/RecordSelectionWidget.cpp \
            $$PWD/Events/UI/RuptureLocation.cpp \
            $$PWD/Events/UI/RuptureWidget.cpp \
            $$PWD/Events/UI/Site.cpp \
            $$PWD/Events/UI/SiteConfig.cpp \
            $$PWD/Events/UI/SiteConfigWidget.cpp \
            $$PWD/Events/UI/SiteGrid.cpp \
            $$PWD/Events/UI/SiteGridWidget.cpp \
            $$PWD/Events/UI/SiteScatter.cpp \
            $$PWD/Events/UI/SiteScatterWidget.cpp \
            $$PWD/Events/UI/SiteWidget.cpp \
            $$PWD/Events/UI/SoilModel.cpp \
            $$PWD/Events/UI/SoilModelWidget.cpp \
            $$PWD/Events/UI/SpatialCorrelationWidget.cpp \
            $$PWD/Events/UI/Vs30.cpp \
            $$PWD/Events/UI/Vs30Widget.cpp \
            $$PWD/ModelViewItems/ComponentTableModel.cpp \
            $$PWD/ModelViewItems/ComponentTableView.cpp \
            $$PWD/ModelViewItems/ListTreeModel.cpp \
            $$PWD/ModelViewItems/CustomListWidget.cpp \
            $$PWD/Tools/AssetInputDelegate.cpp \
            $$PWD/Tools/AssetFilterDelegate.cpp \
            $$PWD/Tools/ComponentDatabase.cpp \
            $$PWD/Tools/CSVReaderWriter.cpp \
            $$PWD/Tools/ComponentDatabaseManager.cpp \
            $$PWD/Tools/NGAW2Converter.cpp \
            $$PWD/Tools/PelicunPostProcessor.cpp \
            $$PWD/Tools/CBCitiesPostProcessor.cpp \
            $$PWD/Tools/REmpiricalProbabilityDistribution.cpp \
            $$PWD/Tools/TablePrinter.cpp \
            $$PWD/Tools/XMLAdaptor.cpp \
            $$PWD/UIWidgets/AnalysisWidget.cpp \
            $$PWD/UIWidgets/AssetsWidget.cpp \
            $$PWD/UIWidgets/BuildingDMEQWidget.cpp \
            $$PWD/UIWidgets/BuildingDMWidget.cpp \
            $$PWD/UIWidgets/BuildingEDPEQWidget.cpp \
            $$PWD/UIWidgets/BuildingEDPWidget.cpp \
            $$PWD/UIWidgets/ColorDialogDelegate.cpp \
            $$PWD/UIWidgets/BuildingModelGeneratorWidget.cpp \
            $$PWD/UIWidgets/BuildingModelingWidget.cpp \
            $$PWD/UIWidgets/BuildingSimulationWidget.cpp \
            $$PWD/UIWidgets/CSVtoBIMModelingWidget.cpp \
            $$PWD/UIWidgets/AssetInputWidget.cpp \
            $$PWD/UIWidgets/NonselectableComponentInputWidget.cpp \
            $$PWD/UIWidgets/DLWidget.cpp \
            $$PWD/UIWidgets/EarthquakeInputWidget.cpp \
            $$PWD/UIWidgets/GeneralInformationWidgetR2D.cpp \
            $$PWD/UIWidgets/GroundMotionStation.cpp \
            $$PWD/UIWidgets/LoadResultsDialog.cpp \
            $$PWD/UIWidgets/ToolDialog.cpp \
            $$PWD/UIWidgets/SimCenterUnitsWidget.cpp \
            $$PWD/UIWidgets/VerticalScrollingWidget.cpp \
            $$PWD/UIWidgets/WindFieldStation.cpp \
            $$PWD/UIWidgets/GroundMotionTimeHistory.cpp \
            $$PWD/UIWidgets/HazardToAssetBuilding.cpp \
            $$PWD/UIWidgets/HazardToAssetWidget.cpp \
            $$PWD/UIWidgets/HazardsWidget.cpp \
            $$PWD/UIWidgets/HousingUnitAllocationWidget.cpp \
            $$PWD/UIWidgets/HurricaneParameterWidget.cpp \
            $$PWD/UIWidgets/InputWidgetOpenSeesPyAnalysis.cpp \
            $$PWD/UIWidgets/ModelWidget.cpp \
            $$PWD/UIWidgets/MultiComponentR2D.cpp \
            $$PWD/UIWidgets/NearestNeighbourMapping.cpp \
            $$PWD/UIWidgets/SiteSpecifiedMapping.cpp \
            $$PWD/UIWidgets/OpenSeesPyBuildingModel.cpp \
            $$PWD/UIWidgets/WaterNetworkPerformanceModel.cpp \
            $$PWD/UIWidgets/PelicunDLWidget.cpp \
            $$PWD/UIWidgets/PerformanceWidget.cpp \
            $$PWD/UIWidgets/ResultsWidget.cpp \
            $$PWD/UIWidgets/SecondaryComponentSelection.cpp \
            $$PWD/UIWidgets/ShakeMapWidget.cpp \
            $$PWD/UIWidgets/SimCenterEventRegional.cpp \
            $$PWD/UIWidgets/StructuralModelingWidget.cpp \
            $$PWD/UIWidgets/UQWidget.cpp \
            $$PWD/UIWidgets/UserDefinedEDPR.cpp \
            $$PWD/UIWidgets/UserInputGMWidget.cpp \
            $$PWD/UIWidgets/RegionalSiteResponseWidget.cpp \
            $$PWD/UIWidgets/HurricaneSelectionWidget.cpp \
            $$PWD/UIWidgets/OpenQuakeSelectionWidget.cpp \
            $$PWD/UIWidgets/UserInputHurricaneWidget.cpp \
            $$PWD/GraphicElements/NodeHandle.cpp \
            $$PWD/GraphicElements/RectangleGrid.cpp \
            $$PWD/GraphicElements/GridNode.cpp \
            $$PWD/RunWidget.cpp \
            $$PWD/WorkflowAppR2D.cpp \

HEADERS +=  $$PWD/Events/UI/EarthquakeRuptureForecast.h \
            $$PWD/Events/UI/BedrockDepth.h \
            $$PWD/Events/UI/BedrockDepthWidget.h \
            $$PWD/Events/UI/EarthquakeRuptureForecastWidget.h \
            $$PWD/Events/UI/EventGMDirWidget.h \
            $$PWD/Events/UI/GMPE.h \
            $$PWD/Events/UI/GMPEWidget.h \
            $$PWD/Events/UI/GMWidget.h \
            $$PWD/Events/UI/GmAppConfig.h \
            $$PWD/Events/UI/GmAppConfigWidget.h \
            $$PWD/Events/UI/GmCommon.h \
            $$PWD/Events/UI/GridDivision.h \
            $$PWD/Events/UI/HBoxFormLayout.h \
            $$PWD/Events/UI/HazardOccurrence.h \
            $$PWD/Events/UI/HazardOccurrenceWidget.h \
            $$PWD/Events/UI/IntensityMeasure.h \
            $$PWD/Events/UI/IntensityMeasureWidget.h \
            $$PWD/Events/UI/Location.h \
            $$PWD/Events/UI/OpenQuakeClassical.h \
            $$PWD/Events/UI/OpenQuakeClassicalWidget.h \
            $$PWD/Events/UI/OpenQuakeScenario.h \
            $$PWD/Events/UI/OpenQuakeScenarioWidget.h \
            $$PWD/Events/UI/OpenQuakeUserSpecifiedWidget.h \
            $$PWD/Events/UI/PeerLoginDialog.h \
            $$PWD/Events/UI/PeerNGAWest2Client.h \
            $$PWD/Events/UI/PointSourceRupture.h \
            $$PWD/Events/UI/PointSourceRuptureWidget.h \
            $$PWD/Events/UI/QGISSiteInputWidget.h \
            $$PWD/Events/UI/RecordSelectionConfig.h \
            $$PWD/Events/UI/RecordSelectionWidget.h \
            $$PWD/Events/UI/RuptureLocation.h \
            $$PWD/Events/UI/RuptureWidget.h \
            $$PWD/Events/UI/Site.h \
            $$PWD/Events/UI/SiteConfig.h \
            $$PWD/Events/UI/SiteConfigWidget.h \
            $$PWD/Events/UI/SiteGrid.h \
            $$PWD/Events/UI/SiteGridWidget.h \
            $$PWD/Events/UI/SiteScatter.h \
            $$PWD/Events/UI/SiteScatterWidget.h \
            $$PWD/Events/UI/SiteWidget.h \
            $$PWD/Events/UI/SoilModel.h \
            $$PWD/Events/UI/SoilModelWidget.h \
            $$PWD/Events/UI/SpatialCorrelationWidget.h \
            $$PWD/Events/UI/Vs30.h \
            $$PWD/Events/UI/Vs30Widget.h \
            $$PWD/Tools/AssetInputDelegate.h \
            $$PWD/Tools/AssetFilterDelegate.h \
            $$PWD/Tools/ComponentDatabase.h \
            $$PWD/Tools/CSVReaderWriter.h \
            $$PWD/Tools/ComponentDatabaseManager.h \
            $$PWD/Tools/NGAW2Converter.h \
            $$PWD/Tools/PelicunPostProcessor.h \
            $$PWD/Tools/CBCitiesPostProcessor.h \
            $$PWD/Tools/REmpiricalProbabilityDistribution.h \
            $$PWD/Tools/TableNumberItem.h \
            $$PWD/Tools/TablePrinter.h \
            $$PWD/Tools/XMLAdaptor.h \
            $$PWD/UIWidgets/AnalysisWidget.h \
            $$PWD/UIWidgets/AssetsWidget.h \
            $$PWD/UIWidgets/BuildingDMEQWidget.h \
            $$PWD/UIWidgets/BuildingDMWidget.h \
            $$PWD/UIWidgets/BuildingEDPEQWidget.h \
            $$PWD/UIWidgets/BuildingEDPWidget.h \
            $$PWD/UIWidgets/ColorDialogDelegate.h \
            $$PWD/UIWidgets/GISObjectTypeMapping.h \
            $$PWD/UIWidgets/BuildingModelGeneratorWidget.h \
            $$PWD/UIWidgets/BuildingModelingWidget.h \
            $$PWD/UIWidgets/BuildingSimulationWidget.h \
            $$PWD/UIWidgets/CSVtoBIMModelingWidget.h \
            $$PWD/UIWidgets/AssetInputWidget.h \
            $$PWD/UIWidgets/NonselectableComponentInputWidget.h \
            $$PWD/UIWidgets/DLWidget.h \
            $$PWD/UIWidgets/EarthquakeInputWidget.h \
            $$PWD/UIWidgets/GeneralInformationWidgetR2D.h \
            $$PWD/UIWidgets/GroundMotionStation.h \
            $$PWD/UIWidgets/LoadResultsDialog.h \
            $$PWD/UIWidgets/ToolDialog.h \
            $$PWD/UIWidgets/SimCenterUnitsWidget.h \
            $$PWD/UIWidgets/VerticalScrollingWidget.h \
            $$PWD/UIWidgets/WindFieldStation.h \
            $$PWD/UIWidgets/GroundMotionTimeHistory.h \
            $$PWD/UIWidgets/HazardToAssetBuilding.h \
            $$PWD/UIWidgets/HazardToAssetWidget.h \
            $$PWD/UIWidgets/HazardsWidget.h \
            $$PWD/UIWidgets/HousingUnitAllocationWidget.h \
            $$PWD/UIWidgets/HurricaneParameterWidget.h \
            $$PWD/UIWidgets/InputWidgetOpenSeesPyAnalysis.h \
            $$PWD/UIWidgets/ModelWidget.h \
            $$PWD/UIWidgets/MultiComponentR2D.h \
            $$PWD/UIWidgets/NearestNeighbourMapping.h \
            $$PWD/UIWidgets/SiteSpecifiedMapping.h \
            $$PWD/UIWidgets/OpenSeesPyBuildingModel.h \
            $$PWD/UIWidgets/WaterNetworkPerformanceModel.h \
            $$PWD/UIWidgets/PelicunDLWidget.h \
            $$PWD/UIWidgets/PerformanceWidget.h \
            $$PWD/UIWidgets/ResultsWidget.h \
            $$PWD/UIWidgets/SecondaryComponentSelection.h \
            $$PWD/UIWidgets/ShakeMapWidget.h \
            $$PWD/UIWidgets/SimCenterEventRegional.h \
            $$PWD/UIWidgets/StructuralModelingWidget.h \
            $$PWD/UIWidgets/UQWidget.h \
            $$PWD/UIWidgets/UserDefinedEDPR.h \
            $$PWD/UIWidgets/UserInputGMWidget.h \
            $$PWD/UIWidgets/RegionalSiteResponseWidget.h \
            $$PWD/UIWidgets/HurricaneSelectionWidget.h \
            $$PWD/UIWidgets/OpenQuakeSelectionWidget.h \
            $$PWD/UIWidgets/UserInputHurricaneWidget.h \
            $$PWD/UIWidgets/HurricaneObject.h \
            $$PWD/ModelViewItems/CustomListWidget.h \
            $$PWD/ModelViewItems/ComponentTableModel.h \
            $$PWD/ModelViewItems/ComponentTableView.h \
            $$PWD/ModelViewItems/ListTreeModel.h \
            $$PWD/GraphicElements/GridNode.h \
            $$PWD/GraphicElements/NodeHandle.h \
            $$PWD/GraphicElements/RectangleGrid.h \
            $$PWD/WorkflowAppR2D.h \
            $$PWD/RunWidget.h \


contains(DEFINES, ARC_GIS)  {

SOURCES +=  $$PWD/ModelViewItems/ArcGISLegendView.cpp \
            $$PWD/GraphicElements/ConvexHull.cpp \
            $$PWD/GraphicElements/PolygonBoundary.cpp \
            $$PWD/ModelViewItems/LayerTreeView.cpp \
            $$PWD/UIWidgets/ArcGISBuildingInputWidget.cpp \
            $$PWD/UIWidgets/ArcGISGasPipelineInputWidget.cpp \
            $$PWD/Tools/ArcGISHurricanePreprocessor.cpp \
            $$PWD/UIWidgets/ArcGISHurricaneSelectionWidget.cpp \
            $$PWD/UIWidgets/LayerManagerDialog.cpp \
            $$PWD/UIWidgets/LayerManagerTableView.cpp \
            $$PWD/UIWidgets/LayerManagerModel.cpp \
            $$PWD/UIWidgets/LayerComboBoxItemDelegate.cpp \
            $$PWD/UIWidgets/RendererModel.cpp \
            $$PWD/UIWidgets/RendererTableView.cpp \
            $$PWD/UIWidgets/RendererComboBoxItemDelegate.cpp \
            $$PWD/UIWidgets/SimCenterMapGraphicsView.cpp \
            $$PWD/UIWidgets/EmbeddedMapViewWidget.cpp \
            $$PWD/UIWidgets/MapViewSubWidget.cpp \

HEADERS +=  $$PWD/ModelViewItems/ArcGISLegendView.h \
            $$PWD/UIWidgets/ArcGISVisualizationWidget.h \
            $$PWD/GraphicElements/ConvexHull.h \
            $$PWD/GraphicElements/PolygonBoundary.h \
            $$PWD/ModelViewItems/LayerTreeView.h \
            $$PWD/Tools/ArcGISHurricanePreprocessor.h \
            $$PWD/UIWidgets/ArcGISBuildingInputWidget.h \
            $$PWD/UIWidgets/ArcGISGasPipelineInputWidget.h \
            $$PWD/UIWidgets/ArcGISHurricaneSelectionWidget.h \
            $$PWD/UIWidgets/LayerManagerDialog.h \
            $$PWD/UIWidgets/LayerManagerTableView.h \
            $$PWD/UIWidgets/LayerManagerModel.h \
            $$PWD/UIWidgets/LayerComboBoxItemDelegate.h \
            $$PWD/UIWidgets/RendererModel.h \
            $$PWD/UIWidgets/RendererTableView.h \
            $$PWD/UIWidgets/RendererComboBoxItemDelegate.h \
            $$PWD/UIWidgets/SimCenterMapGraphicsView.h \
            $$PWD/UIWidgets/EmbeddedMapViewWidget.h \
            $$PWD/UIWidgets/MapViewSubWidget.h \
}


contains(DEFINES, Q_GIS)  {

SOURCES +=  $$PWD/Tools/QGISHurricanePreprocessor.cpp \
            $$PWD/UIWidgets/LineAssetInputWidget.cpp \
            $$PWD/UIWidgets/PointAssetInputWidget.cpp \
            $$PWD/UIWidgets/CSVWaterNetworkInputWidget.cpp \
            $$PWD/UIWidgets/RasterHazardInputWidget.cpp \
            $$PWD/UIWidgets/GISHazardInputWidget.cpp \
            $$PWD/UIWidgets/QGISHurricaneSelectionWidget.cpp \
            $$PWD/UIWidgets/GISAssetInputWidget.cpp \
            $$PWD/UIWidgets/GISWaterNetworkInputWidget.cpp \
            $$PWD/UIWidgets/MapViewWindow.cpp \
            $$PWD/UIWidgets/CRSSelectionWidget.cpp \

HEADERS +=  $$PWD/Tools/QGISHurricanePreprocessor.h \
            $$PWD/UIWidgets/LineAssetInputWidget.h \
            $$PWD/UIWidgets/PointAssetInputWidget.h \
            $$PWD/UIWidgets/CSVWaterNetworkInputWidget.h \
            $$PWD/UIWidgets/RasterHazardInputWidget.h \
            $$PWD/UIWidgets/GISHazardInputWidget.h \
            $$PWD/UIWidgets/QGISHurricaneSelectionWidget.h \
            $$PWD/UIWidgets/GISAssetInputWidget.h \
            $$PWD/UIWidgets/GISWaterNetworkInputWidget.h \
            $$PWD/UIWidgets/MapViewWindow.h \
            $$PWD/UIWidgets/CRSSelectionWidget.h \

}



