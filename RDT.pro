QT += core gui charts concurrent network sql qml webenginewidgets webengine webchannel 3dcore 3drender 3dextras charts xml \
    quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Information about the app
TARGET = RDT
TEMPLATE = app
VERSION=1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# C++14 support
CONFIG += c++17

# Specify the paths to the Simcenter common directory and to groundmotion utilities
PATH_TO_COMMON=../../SimCenterCommon
PATH_TO_GMUTILS=../../GroundMotionUtilitiesCustom

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
include(EarthquakeEvents.pri)

# RDT files
INCLUDEPATH += $$PWD/Utils \
               $$PWD/styles \
               $$PWD/UIWidgets \


SOURCES +=  main.cpp \
            CSVReaderWriter.cpp \
            UIWidgets/CustomGraphicsScene.cpp \
            UIWidgets/CustomGraphicsView.cpp \
            UIWidgets/GridNode.cpp \
            UIWidgets/MapViewSubWidget.cpp \
            UIWidgets/NodeHandle.cpp \
            UIWidgets/RectangleGrid.cpp \
            WorkflowAppRDT.cpp \
            RunWidget.cpp \
            XMLAdaptor.cpp \
            shakeMapClient.cpp \
            UIWidgets/EarthquakeInputWidget.cpp \
            UIWidgets/PopUpWidget.cpp \
            UIWidgets/TreeItem.cpp \
            UIWidgets/TreeModel.cpp \
            UIWidgets/TreeView.cpp \
            UIWidgets/TreeViewStyle.cpp \
            UIWidgets/VisualizationWidget.cpp \
            UIWidgets/ComponentInputWidget.cpp \
            UIWidgets/BuildingModelingWidget.cpp \
            UIWidgets/CSVtoBIMModelingWidget.cpp \
            UIWidgets/DecisionVariableWidget.cpp \
            UIWidgets/HazardsWidget.cpp \
            UIWidgets/ResultsWidget.cpp \
            UIWidgets/AssetsWidget.cpp \
            UIWidgets/AssetsModelWidget.cpp \
            UIWidgets/EngDemandParamWidget.cpp \
            UIWidgets/GeneralInformationWidget.cpp \
            UIWidgets/RegionalMappingWidget.cpp \
            UIWidgets/ShakeMapWidget.cpp \
            UIWidgets/UserInputGMWidget.cpp \
#            UIWidgets/IntensityMeasureWidget.cpp \
            UIWidgets/DamageMeasureWidget.cpp \


HEADERS +=  WorkflowAppRDT.h\
            RunWidget.h \
            UIWidgets/CustomGraphicsScene.h \
            UIWidgets/CustomGraphicsView.h \
            UIWidgets/GridNode.h \
            UIWidgets/MapViewSubWidget.h \
            UIWidgets/NodeHandle.h \
            UIWidgets/RectangleGrid.h \
            XMLAdaptor.h \
            shakeMapClient.h \
            CSVReaderWriter.h \
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
            UIWidgets/CSVtoBIMModelingWidget.h \
            UIWidgets/EngDemandParamWidget.h \
            UIWidgets/GeneralInformationWidget.h \
            UIWidgets/RegionalMappingWidget.h \
            UIWidgets/ShakeMapWidget.h \
            UIWidgets/UserInputGMWidget.h \
#            UIWidgets/IntensityMeasureWidget.h \
            UIWidgets/DamageMeasureWidget.h \


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

win32:INCLUDEPATH += "..\jansson\build\include"
win32:LIBS += "..\jansson\build\lib\release\jansson.lib"


