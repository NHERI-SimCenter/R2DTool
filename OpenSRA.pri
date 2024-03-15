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



INCLUDEPATH += $$PATH_TO_OPENSRA/ModelViewItems \
               $$PATH_TO_OPENSRA/UIWidgets \
               $$PATH_TO_OPENSRA/JsonWidgets \



SOURCES +=  $$PATH_TO_OPENSRA/OpenSRAPreferences.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/PipelineNetworkWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/PipelineDLWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/RandomVariablesWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentDMWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentDVWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentEDPWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/WidgetFactory.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/AddToRunListWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/GenericModelWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/DamageMeasureWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/DecisionVariableWidget.cpp \
            $$PATH_TO_OPENSRA/UIWidgets/EngineeringDemandParameterWidget.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/ComboBoxDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/ButtonDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/LineEditDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/LabelDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/StringListDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/RV.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/MixedDelegate.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/RVTableView.cpp \
            $$PATH_TO_OPENSRA/ModelViewItems/RVTableModel.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/SimCenterJsonWidget.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonDefinedWidget.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonWidget.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonCheckBox.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonComboBox.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonLineEdit.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonLabel.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonGroupBoxWidget.cpp \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonStackedWidget.cpp \


HEADERS +=  $$PATH_TO_OPENSRA/OpenSRAPreferences.h \
            $$PATH_TO_OPENSRA/UIWidgets/PipelineNetworkWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/PipelineDLWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/RandomVariablesWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentDMWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentDVWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/MultiComponentEDPWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/WidgetFactory.h \
            $$PATH_TO_OPENSRA/UIWidgets/AddToRunListWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/GenericModelWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/DamageMeasureWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/DecisionVariableWidget.h \
            $$PATH_TO_OPENSRA/UIWidgets/EngineeringDemandParameterWidget.h \
            $$PATH_TO_OPENSRA/ModelViewItems/ComboBoxDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/ButtonDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/LineEditDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/LabelDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/StringListDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/RV.h \
            $$PATH_TO_OPENSRA/ModelViewItems/MixedDelegate.h \
            $$PATH_TO_OPENSRA/ModelViewItems/RVTableView.h \
            $$PATH_TO_OPENSRA/ModelViewItems/RVTableModel.h \
            $$PATH_TO_OPENSRA/JsonWidgets/SimCenterJsonWidget.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonDefinedWidget.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonWidget.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonCheckBox.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonComboBox.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonLineEdit.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonLabel.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonGroupBoxWidget.h \
            $$PATH_TO_OPENSRA/JsonWidgets/JsonStackedWidget.h \


