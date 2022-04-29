QT       -= gui
TARGET    = R2DTest
CONFIG   += console
CONFIG   -= app_bundle


# C++17 support
CONFIG += c++17

DEFINES +=  Q_GIS

PATH_TO_COMMON=../../../SimCenterCommon
PATH_TO_QGIS_PLUGIN=../../../QGISPlugin


QT += widgets testlib charts network xml 3dcore 3drender 3dextras opengl sql concurrent

macos:LIBS += -lcurl -llapack -lblas
linux:LIBS += /usr/lib/libcurl.so

include($$PATH_TO_COMMON/Common/Common.pri)
include($$PATH_TO_COMMON/RandomVariables/RandomVariables.pri)
include($$PATH_TO_QGIS_PLUGIN/QGIS.pri)

include(../R2DCommon.pri)

include(../R2D.pri)


# The test files
SOURCES += \
        $$PWD/R2DUnitTests.cpp \

