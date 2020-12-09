QT += core gui charts concurrent network sql qml webenginewidgets webengine webchannel 3dcore 3drender 3dextras charts xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Information about the app
TARGET = RDT
TEMPLATE = app
VERSION=1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# C++17 support
CONFIG += c++17

# Specify the path to the Simcenter common directory
PATH_TO_COMMON=../../SimCenterCommon

# Application Icons
win32 {
    RC_ICONS = icons/NHERI-EEUQ-Icon.ico
} else {
    mac {
    ICON = icons/NHERI-EEUQ-Icon.icns
    }
}

# GIS library
ARCGIS_RUNTIME_VERSION = 100.9
include($$PWD/arcgisruntime.pri)

# Simcenter dependencies
include($$PATH_TO_COMMON/Common/Common.pri)
include($$PATH_TO_COMMON/RandomVariables/RandomVariables.pri)
include(RDTCommon.pri)

# RDT files
INCLUDEPATH += $$PWD/Utils \
               $$PWD/styles \
               $$PWD/UIWidgets \
               $$PWD/EVENTS/UI \
               $$PWD/TOOLS \


SOURCES +=  main.cpp \
            TOOLS/NGAW2Converter.cpp \
            UIWidgets/GroundMotionStation.cpp \
            UIWidgets/GroundMotionTimeHistory.cpp \
            WorkflowAppRDT.cpp \
            RunWidget.cpp \
            TOOLS/XMLAdaptor.cpp \
            TOOLS/shakeMapClient.cpp \
            TOOLS/CSVReaderWriter.cpp \
            UIWidgets/SecondaryComponentSelection.cpp \
            UIWidgets/MultiComponentRDT.cpp \
            UIWidgets/CustomGraphicsScene.cpp \
            UIWidgets/CustomGraphicsView.cpp \
            UIWidgets/GridNode.cpp \
            UIWidgets/MapViewSubWidget.cpp \
            UIWidgets/NodeHandle.cpp \
            UIWidgets/RectangleGrid.cpp \         
            UIWidgets/EarthquakeInputWidget.cpp \
            UIWidgets/PopUpWidget.cpp \
            UIWidgets/TreeItem.cpp \
            UIWidgets/TreeModel.cpp \
            UIWidgets/TreeView.cpp \
            UIWidgets/TreeViewStyle.cpp \
            UIWidgets/VisualizationWidget.cpp \
            UIWidgets/ComponentInputWidget.cpp \
            UIWidgets/BuildingModelingWidget.cpp \
            UIWidgets/BuildingModelGeneratorWidget.cpp \
            UIWidgets/BuildingEDPWidget.cpp \
            UIWidgets/BuildingEDPEQWidget.cpp \
            UIWidgets/BuildingDMWidget.cpp \
            UIWidgets/BuildingDMEQWidget.cpp \
            UIWidgets/StructuralModelingWidget.cpp \
            UIWidgets/BuildingSimulationWidget.cpp \
            UIWidgets/CSVtoBIMModelingWidget.cpp \
            UIWidgets/DecisionVariableWidget.cpp \
            UIWidgets/HazardsWidget.cpp \
            UIWidgets/ResultsWidget.cpp \
            UIWidgets/AssetsWidget.cpp \
            UIWidgets/AssetsModelWidget.cpp \
            UIWidgets/DamageMeasureWidget.cpp \
            UIWidgets/PelicunDLWidget.cpp \
            UIWidgets/EngDemandParameterWidget.cpp \
            UIWidgets/GeneralInformationWidget.cpp \
            UIWidgets/RegionalMappingWidget.cpp \
            UIWidgets/ShakeMapWidget.cpp \
            UIWidgets/UserInputGMWidget.cpp \
            UIWidgets/UserDefinedEDPR.cpp \
            UIWidgets/OpenSeesPyBuildingModel.cpp \       
            UIWidgets/InputWidgetOpenSeesPyAnalysis.cpp \
            EVENTS/UI/GMWidget.cpp \
            EVENTS/UI/Location.cpp \
            EVENTS/UI/Site.cpp \
            EVENTS/UI/PointSourceRupture.cpp \
            EVENTS/UI/EarthquakeRuptureForecast.cpp \
            EVENTS/UI/RuptureLocation.cpp \
            EVENTS/UI/GMPE.cpp \
            EVENTS/UI/IntensityMeasure.cpp \
            EVENTS/UI/SiteWidget.cpp \
            EVENTS/UI/RuptureWidget.cpp \
            EVENTS/UI/EarthquakeRuptureForecastWidget.cpp \
            EVENTS/UI/PointSourceRuptureWidget.cpp \
            EVENTS/UI/GMPEWidget.cpp \
            EVENTS/UI/IntensityMeasureWidget.cpp \
            EVENTS/UI/SpatialCorrelationWidget.cpp \
            EVENTS/UI/RecordSelectionConfig.cpp \
            EVENTS/UI/RecordSelectionWidget.cpp \
            EVENTS/UI/HBoxFormLayout.cpp \
            EVENTS/UI/SiteGrid.cpp \
            EVENTS/UI/GridDivision.cpp \
            EVENTS/UI/SiteConfig.cpp \
            EVENTS/UI/SiteConfigWidget.cpp \
            EVENTS/UI/SiteGridWidget.cpp \
            EVENTS/UI/GmAppConfig.cpp \
            EVENTS/UI/GmAppConfigWidget.cpp \
            EVENTS/UI/GmCommon.cpp \
            EVENTS/UI/PeerLoginDialog.cpp \
            EVENTS/UI/PeerNGAWest2Client.cpp


HEADERS +=  WorkflowAppRDT.h\
            PEERUserPass.h \
            RunWidget.h \
            TOOLS/NGAW2Converter.h \
            TOOLS/XMLAdaptor.h \
            TOOLS/shakeMapClient.h \
            TOOLS/CSVReaderWriter.h \
            UIWidgets/GroundMotionStation.h \
            UIWidgets/GroundMotionTimeHistory.h \
            UIWidgets/SecondaryComponentSelection.h \
            UIWidgets/MultiComponentRDT.h \            
            UIWidgets/CustomGraphicsScene.h \
            UIWidgets/CustomGraphicsView.h \
            UIWidgets/GridNode.h \
            UIWidgets/MapViewSubWidget.h \
            UIWidgets/NodeHandle.h \
            UIWidgets/RectangleGrid.h \
            UIWidgets/EarthquakeInputWidget.h \
            UIWidgets/PopUpWidget.h \
            UIWidgets/TreeItem.h \
            UIWidgets/TreeModel.h \
            UIWidgets/TreeView.h \
            UIWidgets/TreeViewStyle.h \
            UIWidgets/DecisionVariableWidget.h \
            UIWidgets/HazardsWidget.h \
            UIWidgets/ResultsWidget.h \
            UIWidgets/AssetsWidget.h \
            UIWidgets/AssetsModelWidget.h \
            UIWidgets/VisualizationWidget.h \
            UIWidgets/ComponentInputWidget.h \
            UIWidgets/BuildingModelingWidget.h \
            UIWidgets/StructuralModelingWidget.h \
            UIWidgets/BuildingModelGeneratorWidget.h \
            UIWidgets/BuildingEDPWidget.h \
            UIWidgets/BuildingEDPEQWidget.h \
            UIWidgets/BuildingDMWidget.h \
            UIWidgets/BuildingDMEQWidget.h \
            UIWidgets/BuildingSimulationWidget.h \
            UIWidgets/CSVtoBIMModelingWidget.h \
            UIWidgets/DamageMeasureWidget.h \
            UIWidgets/PelicunDLWidget.h \
            UIWidgets/EngDemandParameterWidget.h \
            UIWidgets/GeneralInformationWidget.h \
            UIWidgets/RegionalMappingWidget.h \
            UIWidgets/ShakeMapWidget.h \
            UIWidgets/UserInputGMWidget.h \
            UIWidgets/UserDefinedEDPR.h \
            UIWidgets/OpenSeesPyBuildingModel.h \
            UIWidgets/InputWidgetOpenSeesPyAnalysis.h \
            EVENTS/UI/GMWidget.h \
            EVENTS/UI/Location.h \
            EVENTS/UI/Site.h \
            EVENTS/UI/PointSourceRupture.h \
            EVENTS/UI/EarthquakeRuptureForecast.h \
            EVENTS/UI/RuptureLocation.h \
            EVENTS/UI/EarthquakeRuptureForecastWidget.h \
            EVENTS/UI/PointSourceRuptureWidget.h \
            EVENTS/UI/GMPE.h \
            EVENTS/UI/IntensityMeasure.h \
            EVENTS/UI/SiteWidget.h \
            EVENTS/UI/RuptureWidget.h \
            EVENTS/UI/GMPEWidget.h \
            EVENTS/UI/IntensityMeasureWidget.h \
            EVENTS/UI/SpatialCorrelationWidget.h \
            EVENTS/UI/JsonSerializable.h \
            EVENTS/UI/RecordSelectionConfig.h \
            EVENTS/UI/RecordSelectionWidget.h \
            EVENTS/UI/HBoxFormLayout.h \
            EVENTS/UI/SiteGrid.h \
            EVENTS/UI/GridDivision.h \
            EVENTS/UI/SiteConfig.h \
            EVENTS/UI/SiteConfigWidget.h \
            EVENTS/UI/SiteGridWidget.h \
            EVENTS/UI/GmAppConfig.h \
            EVENTS/UI/GmAppConfigWidget.h \
            EVENTS/UI/GmCommon.h \
            EVENTS/UI/PeerLoginDialog.h \
            EVENTS/UI/PeerNGAWest2Client.h


RESOURCES += \
    images.qrc \
    $$PWD/styles.qrc


DISTFILES += \
    resources/docs/textAboutEEUQ.html

# External libraries
macos:LIBS += /usr/lib/libcurl.dylib -llapack -lblas
win32:INCLUDEPATH += "c:\Users\SimCenter\libCurl-7.59.0\include"
win32:LIBS += C:\Users\SimCenter\libCurl-7.59.0/lib/libcurl.lib
linux:LIBS += /usr/lib/x86_64-linux-gnu/libcurl.so

#win32:INCLUDEPATH += "..\jansson\build\include"
#win32:LIBS += "..\jansson\build\lib\release\jansson.lib"


