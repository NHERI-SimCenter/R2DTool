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
               $$PWD/assetWidgets \
               $$PWD/SystemPerformanceWidgets \
	       $$PWD/RecoveryWidgets \
	       $$PWD/RecoveryWidgets/pyrecodes \
               $$PWD/assetWidgets/EPANET2.2/include \
	       $$PWD/assetWidgets/EPANET2.2/src \
	       $$PWD/dlWidgets

SOURCES +=  $$PWD/Events/UI/BedrockDepth.cpp \
            $$PWD/Events/UI/BedrockDepthWidget.cpp \
    $$PWD/Events/UI/GroundFailureWidget.cpp \
    $$PWD/Events/UI/LandslideBrayMacedo2019.cpp \
    $$PWD/Events/UI/LandslideWidget.cpp \
    $$PWD/Events/UI/LiqLateralHazus2020.cpp \
    $$PWD/Events/UI/LiqTriggerHazus2020.cpp \
    $$PWD/Events/UI/LiqVerticalHazus2020.cpp \
    $$PWD/Events/UI/LiquefactionWidget.cpp \
            $$PWD/Events/UI/UCERF2Widget.cpp \
            $$PWD/Events/UI/HazardConsistentScenarioWidget.cpp \
            $$PWD/Events/UI/ConventionalScenarioWidget.cpp \
            $$PWD/Events/UI/SpecificScenarioWidget.cpp \
            $$PWD/Events/UI/MeanUCERFWidget.cpp \
            $$PWD/Events/UI/MeanUCERFFM3Widget.cpp \
            $$PWD/Events/UI/MeanUCERFPoissonWidget.cpp \
            $$PWD/Events/UI/HazardCurveInputWidget.cpp \
            $$PWD/Events/UI/EventGMDirWidget.cpp \
            $$PWD/Events/UI/GMPE.cpp \
            $$PWD/Events/UI/GMPEWidget.cpp \
            $$PWD/Events/UI/GMWidget.cpp \
            $$PWD/Events/UI/ScenarioSelectionWidget.cpp \
            $$PWD/Events/UI/GroundMotionModelsWidget.cpp \
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
            $$PWD/Events/UI/NSHMCurveWidget.cpp \
            $$PWD/Events/UI/UserDefinedCurveWidget.cpp \
            $$PWD/Events/UI/Site.cpp \
            $$PWD/Events/UI/SiteConfig.cpp \
            $$PWD/Events/UI/SiteConfigWidget.cpp \
            $$PWD/Events/UI/SiteGrid.cpp \
            $$PWD/Events/UI/SiteGridWidget.cpp \
            $$PWD/Events/UI/SiteScatter.cpp \
            $$PWD/Events/UI/SiteScatterWidget.cpp \
            $$PWD/Events/UI/SiteWidget.cpp \
            $$PWD/Events/UI/GMSiteWidget.cpp \
            $$PWD/Events/UI/GMERFWidget.cpp \
            $$PWD/Events/UI/SoilModel.cpp \
            $$PWD/Events/UI/SoilModelWidget.cpp \
            $$PWD/Events/UI/SpatialCorrelationWidget.cpp \
            $$PWD/Events/UI/Vs30.cpp \
            $$PWD/Events/UI/Vs30Widget.cpp \
    $$PWD/Events/UI/LiqTriggerZhuEtAl2017.cpp \
            $$PWD/Events/UI/zDepthWidget.cpp \
            $$PWD/Events/UI/zDepthUserInputWidget.cpp \
            $$PWD/ModelViewItems/ComponentTableModel.cpp \
            $$PWD/ModelViewItems/ComponentTableView.cpp \
            $$PWD/ModelViewItems/ListTreeModel.cpp \
            $$PWD/ModelViewItems/CustomListWidget.cpp \
            $$PWD/Tools/AssetInputDelegate.cpp \
            $$PWD/Tools/AssetFilterDelegate.cpp \
            $$PWD/Tools/ComponentDatabase.cpp \
            $$PWD/Tools/CSVReaderWriter.cpp \
            $$PWD/Tools/GeoJSONReaderWriter.cpp \
            $$PWD/Tools/ComponentDatabaseManager.cpp \
            $$PWD/Tools/NGAW2Converter.cpp \
            $$PWD/Tools/Pelicun3PostProcessor.cpp \
            $$PWD/Tools/PelicunPostProcessor.cpp \
            $$PWD/Tools/CBCitiesPostProcessor.cpp \
            $$PWD/Tools/REmpiricalProbabilityDistribution.cpp \
            $$PWD/Tools/ResidualDemandResults.cpp \
            $$PWD/Tools/TablePrinter.cpp \
            $$PWD/Tools/XMLAdaptor.cpp \
			$$PWD/Tools/LineEditSelectTool.cpp \
            $$PWD/UIWidgets/ANAWidgets/ANACapacitySpectrumWidget.cpp \
            $$PWD/UIWidgets/ANAWidgets/CapacitySpectrumWidgets/HAZUSDemandWidget.cpp \
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
            $$PWD/UIWidgets/GISTransportNetworkInputWidget.cpp \
            $$PWD/UIWidgets/GeojsonAssetInputWidget.cpp \
            $$PWD/UIWidgets/NonselectableComponentInputWidget.cpp \
            $$PWD/UIWidgets/DLWidget.cpp \
            $$PWD/UIWidgets/EarthquakeInputWidget.cpp \
            $$PWD/UIWidgets/GeneralInformationWidgetR2D.cpp \
            $$PWD/UIWidgets/GroundMotionStation.cpp \
            $$PWD/UIWidgets/LoadResultsDialog.cpp \
            $$PWD/UIWidgets/ToolDialog.cpp \
            $$PWD/UIWidgets/SimCenterUnitsWidget.cpp \
            $$PWD/UIWidgets/SimCenterIMWidget.cpp \
            $$PWD/UIWidgets/VerticalScrollingWidget.cpp \
            $$PWD/UIWidgets/WindFieldStation.cpp \
            $$PWD/UIWidgets/GroundMotionTimeHistory.cpp \
            $$PWD/UIWidgets/HazardToAssetWidget.cpp \
            $$PWD/UIWidgets/HazardsWidget.cpp \
            $$PWD/UIWidgets/HousingUnitAllocationWidget.cpp \
            $$PWD/UIWidgets/HurricaneParameterWidget.cpp \
            $$PWD/UIWidgets/InputWidgetOpenSeesPyAnalysis.cpp \
            $$PWD/UIWidgets/ModelWidget.cpp \
            $$PWD/UIWidgets/MultiComponentR2D.cpp \
            $$PWD/UIWidgets/NearestNeighbourMapping.cpp \
            $$PWD/UIWidgets/SiteSpecifiedMapping.cpp \
            $$PWD/UIWidgets/GISBasedMapping.cpp \
            $$PWD/UIWidgets/OpenSeesPyBuildingModel.cpp \
            $$PWD/UIWidgets/SurrogatePyFilter.cpp \
            $$PWD/UIWidgets/WaterNetworkPerformanceModel.cpp \
            $$PWD/UIWidgets/PelicunDLWidget.cpp \
            $$PWD/UIWidgets/Pelicun3DLWidget.cpp \	    
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
            $$PWD/UIWidgets/BrailsInventoryGenerator.cpp \
            $$PWD/UIWidgets/BrailsTranspInventoryGenerator.cpp \	
            $$PWD/UIWidgets/BrailsGoogleDialog.cpp \	    
            $$PWD/UIWidgets/UserInputHurricaneWidget.cpp \
	    $$PWD/UIWidgets/PyReCodesWidget.cpp \
            $$PWD/GraphicElements/NodeHandle.cpp \
            $$PWD/GraphicElements/RectangleGrid.cpp \
            $$PWD/GraphicElements/PlainRectangle.cpp \	    
            $$PWD/GraphicElements/GridNode.cpp \
            $$PWD/GraphicElements/GIS_Selection.cpp \
	    $$PWD/RunWidget.cpp \
            $$PWD/WorkflowAppR2D.cpp \
            $$PWD/SystemPerformanceWidgets/SystemPerformanceWidget.cpp \
            $$PWD/SystemPerformanceWidgets/RewetRecovery.cpp \
            $$PWD/SystemPerformanceWidgets/RewetResults.cpp \
            $$PWD/RecoveryWidgets/RecoveryWidget.cpp \
	    $$PWD/RecoveryWidgets/Pyrecodes.cpp \
	    $$PWD/RecoveryWidgets/pyrecodes/Pyrecodes2.cpp \
	    $$PWD/RecoveryWidgets/pyrecodes/Pyrecodes3.cpp \	    
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesSystemConfig.cpp \
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesComponentLibrary.cpp \
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesComponent.cpp \
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesLocality.cpp \
            $$PWD/assetWidgets/InpFileWaterInputWidget.cpp \
	    $$PWD/assetWidgets/EPANET2.2/geoJSON.c \
	    $$PWD/assetWidgets/EPANET2.2/src/inpfile.c \
	    $$PWD/assetWidgets/EPANET2.2/src/qualreact.c \
	    $$PWD/assetWidgets/EPANET2.2/src/genmmd.c \
	    $$PWD/assetWidgets/EPANET2.2/src/hydcoeffs.c \
	    $$PWD/assetWidgets/EPANET2.2/src/input1.c \
	    $$PWD/assetWidgets/EPANET2.2/src/output.c \
	    $$PWD/assetWidgets/EPANET2.2/src/qualroute.c \
	    $$PWD/assetWidgets/EPANET2.2/src/epanet.c \
	    $$PWD/assetWidgets/EPANET2.2/src/epanet2.c \	    
	    $$PWD/assetWidgets/EPANET2.2/src/hydraul.c \
	    $$PWD/assetWidgets/EPANET2.2/src/input2.c \
	    $$PWD/assetWidgets/EPANET2.2/src/outputJSON.c \
	    $$PWD/assetWidgets/EPANET2.2/src/report.c \
	    $$PWD/assetWidgets/EPANET2.2/src/hydsolver.c \
	    $$PWD/assetWidgets/EPANET2.2/src/input3.c \
	    $$PWD/assetWidgets/EPANET2.2/src/project.c \
	    $$PWD/assetWidgets/EPANET2.2/src/rules.c \
	    $$PWD/assetWidgets/EPANET2.2/src/hash.c \
	    $$PWD/assetWidgets/EPANET2.2/src/hydstatus.c \
	    $$PWD/assetWidgets/EPANET2.2/src/mempool.c \
	    $$PWD/assetWidgets/EPANET2.2/src/quality.c \
	    $$PWD/assetWidgets/EPANET2.2/src/smatrix.c

HEADERS +=  $$PWD/Events/UI/BedrockDepth.h \
            $$PWD/Events/UI/BedrockDepthWidget.h \
    $$PWD/Events/UI/GroundFailureWidget.h \
    $$PWD/Events/UI/LandslideBrayMacedo2019.h \
    $$PWD/Events/UI/LandslideWidget.h \
    $$PWD/Events/UI/LiqLateralHazus2020.h \
    $$PWD/Events/UI/LiqTriggerHazus2020.h \
    $$PWD/Events/UI/LiqVerticalHazus2020.h \
    $$PWD/Events/UI/LiquefactionWidget.h \
            $$PWD/Events/UI/UCERF2Widget.h \
            $$PWD/Events/UI/HazardConsistentScenarioWidget.h \
            $$PWD/Events/UI/ConventionalScenarioWidget.h \
            $$PWD/Events/UI/SpecificScenarioWidget.h \
            $$PWD/Events/UI/MeanUCERFWidget.h \
            $$PWD/Events/UI/MeanUCERFFM3Widget.h \
            $$PWD/Events/UI/MeanUCERFPoissonWidget.h \
            $$PWD/Events/UI/HazardCurveInputWidget.h \
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
            $$PWD/Events/UI/NSHMCurveWidget.h \
            $$PWD/Events/UI/UserDefinedCurveWidget.h \
            $$PWD/Events/UI/Site.h \
            $$PWD/Events/UI/SiteConfig.h \
            $$PWD/Events/UI/SiteConfigWidget.h \
            $$PWD/Events/UI/SiteGrid.h \
            $$PWD/Events/UI/SiteGridWidget.h \
            $$PWD/Events/UI/SiteScatter.h \
            $$PWD/Events/UI/SiteScatterWidget.h \
            $$PWD/Events/UI/SiteWidget.h \
            $$PWD/Events/UI/GMSiteWidget.h \
            $$PWD/Events/UI/GMERFWidget.h \
            $$PWD/Events/UI/SoilModel.h \
            $$PWD/Events/UI/SoilModelWidget.h \
            $$PWD/Events/UI/SpatialCorrelationWidget.h \
            $$PWD/Events/UI/Vs30.h \
            $$PWD/Events/UI/Vs30Widget.h \
            $$PWD/Events/UI/LiqTriggerZhuEtAl2017.h \
            $$PWD/Events/UI/zDepthWidget.h \
            $$PWD/Events/UI/zDepthUserInputWidget.h \
            $$PWD/Tools/AssetInputDelegate.h \
            $$PWD/Tools/AssetFilterDelegate.h \
            $$PWD/Tools/ComponentDatabase.h \
            $$PWD/Tools/CSVReaderWriter.h \
            $$PWD/Tools/GeoJSONReaderWriter.h \
            $$PWD/Tools/ComponentDatabaseManager.h \
            $$PWD/Tools/NGAW2Converter.h \
            $$PWD/Tools/Pelicun3PostProcessor.h \
            $$PWD/Tools/PelicunPostProcessor.h \
            $$PWD/Tools/CBCitiesPostProcessor.h \
            $$PWD/Tools/REmpiricalProbabilityDistribution.h \
            $$PWD/Tools/ResidualDemandResults.h \
            $$PWD/Tools/TableNumberItem.h \
            $$PWD/Tools/TablePrinter.h \
            $$PWD/Tools/XMLAdaptor.h \
			$$PWD/Tools/LineEditSelectTool.h \
            $$PWD/UIWidgets/ANAWidgets/ANACapacitySpectrumWidget.h \
            $$PWD/UIWidgets/ANAWidgets/CapacitySpectrumWidgets/HAZUSDemandWidget.h \
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
            $$PWD/UIWidgets/GISTransportNetworkInputWidget.h \
            $$PWD/UIWidgets/GeojsonAssetInputWidget.h \
            $$PWD/UIWidgets/NonselectableComponentInputWidget.h \
            $$PWD/UIWidgets/DLWidget.h \
            $$PWD/UIWidgets/EarthquakeInputWidget.h \
            $$PWD/UIWidgets/GeneralInformationWidgetR2D.h \
            $$PWD/UIWidgets/GroundMotionStation.h \
            $$PWD/UIWidgets/LoadResultsDialog.h \
            $$PWD/UIWidgets/ToolDialog.h \
            $$PWD/UIWidgets/SimCenterUnitsWidget.h \
            $$PWD/UIWidgets/SimCenterIMWidget.h \
            $$PWD/UIWidgets/VerticalScrollingWidget.h \
            $$PWD/UIWidgets/WindFieldStation.h \
            $$PWD/UIWidgets/GroundMotionTimeHistory.h \
            $$PWD/UIWidgets/HazardToAssetWidget.h \
            $$PWD/UIWidgets/HazardsWidget.h \
            $$PWD/UIWidgets/HousingUnitAllocationWidget.h \
            $$PWD/UIWidgets/HurricaneParameterWidget.h \
            $$PWD/UIWidgets/InputWidgetOpenSeesPyAnalysis.h \
            $$PWD/UIWidgets/ModelWidget.h \
            $$PWD/UIWidgets/MultiComponentR2D.h \
            $$PWD/UIWidgets/NearestNeighbourMapping.h \
            $$PWD/UIWidgets/SiteSpecifiedMapping.h \
            $$PWD/UIWidgets/GISBasedMapping.h \
            $$PWD/UIWidgets/OpenSeesPyBuildingModel.h \
            $$PWD/UIWidgets/SurrogatePyFilter.h \
            $$PWD/UIWidgets/WaterNetworkPerformanceModel.h \
            $$PWD/UIWidgets/PelicunDLWidget.h \
            $$PWD/UIWidgets/Pelicun3DLWidget.h \	    
            $$PWD/UIWidgets/PerformanceWidget.h \
            $$PWD/UIWidgets/ResultsWidget.h \
            $$PWD/UIWidgets/SecondaryComponentSelection.h \
            $$PWD/UIWidgets/ShakeMapWidget.h \
            $$PWD/UIWidgets/SimCenterEventRegional.h \
            $$PWD/UIWidgets/StructuralModelingWidget.h \
            $$PWD/UIWidgets/UQWidget.h \
            $$PWD/UIWidgets/UserDefinedEDPR.h \
            $$PWD/UIWidgets/UserInputGMWidget.h \
	    $$PWD/UIWidgets/PyReCodesWidget.h \
            $$PWD/Events/UI/ScenarioSelectionWidget.h \
            $$PWD/Events/UI/GroundMotionModelsWidget.h \
            $$PWD/UIWidgets/RegionalSiteResponseWidget.h \
            $$PWD/UIWidgets/HurricaneSelectionWidget.h \
            $$PWD/UIWidgets/OpenQuakeSelectionWidget.h \
            $$PWD/UIWidgets/BrailsInventoryGenerator.h \
            $$PWD/UIWidgets/BrailsTranspInventoryGenerator.h \            
            $$PWD/UIWidgets/BrailsGoogleDialog.h \	    
            $$PWD/UIWidgets/UserInputHurricaneWidget.h \
            $$PWD/UIWidgets/HurricaneObject.h \
            $$PWD/ModelViewItems/CustomListWidget.h \
            $$PWD/ModelViewItems/ComponentTableModel.h \
            $$PWD/ModelViewItems/ComponentTableView.h \
            $$PWD/ModelViewItems/ListTreeModel.h \
            $$PWD/GraphicElements/GridNode.h \
            $$PWD/GraphicElements/NodeHandle.h \
            $$PWD/GraphicElements/RectangleGrid.h \
            $$PWD/GraphicElements/PlainRectangle.h \	    
            $$PWD/GraphicElements/GIS_Selection.h \
            $$PWD/WorkflowAppR2D.h \
            $$PWD/RunWidget.h \
            $$PWD/SystemPerformanceWidgets/SystemPerformanceWidget.h \
            $$PWD/SystemPerformanceWidgets/RewetRecovery.h \
            $$PWD/SystemPerformanceWidgets/RewetResults.h \
            $$PWD/RecoveryWidgets/RecoveryWidget.h \
	    $$PWD/RecoveryWidgets/Pyrecodes.h \
	    $$PWD/RecoveryWidgets/pyrecodes/Pyrecodes2.h \
	    $$PWD/RecoveryWidgets/pyrecodes/Pyrecodes3.h \	    
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesSystemConfig.h \
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesComponentLibrary.h \
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesComponent.h \	    
	    $$PWD/RecoveryWidgets/pyrecodes/PyrecodesLocality.h \	    
            $$PWD/assetWidgets/InpFileWaterInputWidget.h \	    
	    $$PWD/assetWidgets/EPANET2.2/include/epanet2.h \
	    $$PWD/assetWidgets/EPANET2.2/include/epanet2_2.h \
	    $$PWD/assetWidgets/EPANET2.2/include/epanet2_enums.h \
	    $$PWD/assetWidgets/EPANET2.2/src/enumstxt.h \
	    $$PWD/assetWidgets/EPANET2.2/src/funcs.h \
	    $$PWD/assetWidgets/EPANET2.2/src/hash.h \
	    $$PWD/assetWidgets/EPANET2.2/src/text.h \
	    $$PWD/assetWidgets/EPANET2.2/src/types.h \
	    $$PWD/assetWidgets/EPANET2.2/src/version.h \
	    $$PWD/assetWidgets/EPANET2.2/src/mempool.h 

#contains(DEFINES, ARC_GIS)  {

#SOURCES +=  $$PWD/ModelViewItems/ArcGISLegendView.cpp \ d
#            $$PWD/GraphicElements/ConvexHull.cpp \ d
#            $$PWD/GraphicElements/PolygonBoundary.cpp \ d
#            $$PWD/ModelViewItems/LayerTreeView.cpp \
#            $$PWD/UIWidgets/ArcGISBuildingInputWidget.cpp \ d
#            $$PWD/UIWidgets/ArcGISGasPipelineInputWidget.cpp \d
#            $$PWD/Tools/ArcGISHurricanePreprocessor.cpp \ d
#            $$PWD/UIWidgets/ArcGISHurricaneSelectionWidget.cpp \ d
#            $$PWD/UIWidgets/LayerManagerDialog.cpp \ d
#            $$PWD/UIWidgets/LayerManagerTableView.cpp \d
#            $$PWD/UIWidgets/LayerManagerModel.cpp \d
#            $$PWD/UIWidgets/LayerComboBoxItemDelegate.cpp \ d
#            $$PWD/UIWidgets/RendererModel.cpp \d
#            $$PWD/UIWidgets/RendererTableView.cpp \d
#            $$PWD/UIWidgets/RendererComboBoxItemDelegate.cpp \ d
#            $$PWD/UIWidgets/SimCenterMapGraphicsView.cpp \ d
#            $$PWD/UIWidgets/EmbeddedMapViewWidget.cpp \
#            $$PWD/UIWidgets/MapViewSubWidget.cpp \

#HEADERS +=  $$PWD/ModelViewItems/ArcGISLegendView.h \
#            $$PWD/UIWidgets/ArcGISVisualizationWidget.h \
#            $$PWD/GraphicElements/ConvexHull.h \
#            $$PWD/GraphicElements/PolygonBoundary.h \
#            $$PWD/ModelViewItems/LayerTreeView.h \
#            $$PWD/Tools/ArcGISHurricanePreprocessor.h \
#            $$PWD/UIWidgets/ArcGISBuildingInputWidget.h \
#            $$PWD/UIWidgets/ArcGISGasPipelineInputWidget.h \
#            $$PWD/UIWidgets/ArcGISHurricaneSelectionWidget.h \
#            $$PWD/UIWidgets/LayerManagerDialog.h \
#            $$PWD/UIWidgets/LayerManagerTableView.h \
#            $$PWD/UIWidgets/LayerManagerModel.h \
#            $$PWD/UIWidgets/LayerComboBoxItemDelegate.h \
#            $$PWD/UIWidgets/RendererModel.h \
#            $$PWD/UIWidgets/RendererTableView.h \
#            $$PWD/UIWidgets/RendererComboBoxItemDelegate.h \
#            $$PWD/UIWidgets/SimCenterMapGraphicsView.h \
#            $$PWD/UIWidgets/EmbeddedMapViewWidget.h \
#            $$PWD/UIWidgets/MapViewSubWidget.h \
#}


# Add QGIS sources and headers

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
#            $$PWD/UIWidgets/EmbeddedMapViewWidget.cpp \
#            $$PWD/UIWidgets/MapViewSubWidget.cpp \
#            $$PWD/ModelViewItems/LayerTreeView.cpp \

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
#            $$PWD/UIWidgets/EmbeddedMapViewWidget.h \
#            $$PWD/UIWidgets/MapViewSubWidget.h \
#            $$PWD/ModelViewItems/LayerTreeView.h \





