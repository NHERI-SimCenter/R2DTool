#*****************************************************************************
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

# Written by: Stevan Gavrilovic, Frank McKenna

QT += core gui charts concurrent network sql xml serialport webenginewidgets
QT += 3dcore 3drender 3dextras opengl positioning quickwidgets

mac {
QT += macextras
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Information about the app
TARGET = R2D
TEMPLATE = app
VERSION = 1.1.1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"


# Specify the path to the Simcenter common directory
PATH_TO_COMMON=../SimCenterCommon

# Specify the path to the R2D tool examples folder
PATH_TO_EXAMPLES=../R2DExamples


## ADVANCED USAGE BELOW ##

# C++17 support
CONFIG += c++17

# Check for the required Qt version
equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 15) {
        error("$$TARGET requires Qt 5.15.0")
    }
        equals(QT_MINOR_VERSION, 15) : lessThan(QT_PATCH_VERSION, 0) {
                error("$$TARGET requires Qt 5.15.0")
        }
}

DEFINES += Q_GIS

win32:DEFINES +=  CURL_STATICLIB

#win32::include($$PWD/R2D.user.pri)
win32::include($$PWD/ConanHelper.pri)

win32::LIBS+=Advapi32.lib

# Full optimization on release
win32 {
    QMAKE_CXXFLAGS_RELEASE += -O2
} else {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

# Application Icons
win32 {
    RC_ICONS = icons/NHERI-R2D-Icon.ico
} else {
    mac {
    ICON = icons/NHERI-R2D-Icon.icns
    }
}


# GIS library

PATH_TO_QGIS_PLUGIN=../QGISPlugin

message("Building with QGIS library")

include($$PATH_TO_QGIS_PLUGIN/QGIS.pri)


# Simcenter dependencies
include($$PATH_TO_COMMON/Common/Common.pri)
include($$PATH_TO_COMMON/RandomVariables/RandomVariables.pri)
include($$PATH_TO_COMMON/Workflow/Workflow.pri)
include($$PATH_TO_COMMON/InputSheetBM/InputSheetBM.pri)   
   
#include(R2DCommon.pri)

# R2D source and includes
include(R2D.pri)

# Add the main file
SOURCES +=  main.cpp \


# Does this build include the secret user pass for PEER database access? Check if the password file exists.
exists( $$PWD/R2DUserPass.h ) {

    DEFINES += INCLUDE_USER_PASS

} else {
    message( "Warning: PEER password file not found. Look in the file SampleUserPass.h to add your credentials" )
}


contains(DEFINES, INCLUDE_USER_PASS) {

HEADERS += R2DUserPass.h \

} else {

HEADERS += SampleUserPass.h \

}


RESOURCES += \
    images.qrc \
    $$PWD/styles.qrc


DISTFILES += \
    resources/docs/textAboutR2DT.html

# External libraries
macos:LIBS += -lcurl -llapack -lblas
linux:LIBS += /usr/lib/libcurl.so

# Path to build directory
win32 {
DESTDIR = $$shell_path($$OUT_PWD)
Release:DESTDIR = $$DESTDIR/release
Debug:DESTDIR = $$DESTDIR/debug

EXAMPLES_DIR=$$DESTDIR/Examples
DATABASE_DIR=$$DESTDIR/Databases

} else {
    mac {
    EXAMPLES_DIR=$$OUT_PWD/R2D.app/Contents/MacOS/Examples
    DATABASE_DIR=$$OUT_PWD/R2D.app/Contents/MacOS

    mkpath($$OUT_PWD/R2D.app/Contents/MacOS)

    mkpath($$OUT_PWD/R2D.app/Contents/MacOS/Examples)

    }
}

win32 {

# Copies over the examples folder into the build directory
# Copydata.commands = $(COPY_DIR) $$shell_quote($$shell_path($$PATH_TO_EXAMPLES/Examples)) $$shell_quote($$shell_path($$EXAMPLES_DIR))
# first.depends = $(first) Copydata

# Copies the dll files into the build directory
# CopyDLLs.commands = $(COPY_DIR) $$shell_quote($$shell_path($$PWD/winDLLS)) $$shell_quote($$shell_path($$DESTDIR))
# first.depends += CopyDLLs

# CopyDbs.commands = $(COPY_DIR) $$shell_quote($$shell_path($$PWD/Databases)) $$shell_quote($$shell_path($$DATABASE_DIR))
# first.depends += CopyDbs

# export(first.depends)
# export(CopyDbs.commands)
# export(CopyDLLs.commands)

# QMAKE_EXTRA_TARGETS += first Copydata CopyDbs CopyDLLs

}else {
mac {

message($$PATH_TO_EXAMPLES)

exists( $$shell_path($$PATH_TO_EXAMPLES/Examples.json) ) {

# Copies the examples folder into the build directory
Copydata.commands = $(COPY_DIR) $$shell_quote($$shell_path($$PATH_TO_EXAMPLES/Examples.json)) $$shell_quote($$shell_path($$EXAMPLES_DIR))

# Copies the databases folder into the build directory
CopyDbs.commands = $(COPY_DIR) $$shell_quote($$shell_path($$PWD/Databases)) $$shell_quote($$shell_path($$DATABASE_DIR))

first.depends += Copydata CopyDbs

export(first.depends)
export(Copydata.commands)
export(CopyDbs.commands)

QMAKE_EXTRA_TARGETS += first Copydata CopyDbs

} else {
message("Warning: Could not find Examples.json, skipping copy functionality")
}
}
}
