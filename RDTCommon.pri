#--------------------------------------#
#                                      #
# Project created by Stevan Gavrilovic #
#                                      #
#  University of California, Berkeley  #
#--------------------------------------#

INCLUDEPATH +=  $$PATH_TO_COMMON/Workflow/UQ \
                $$PATH_TO_COMMON/Workflow/WORKFLOW \
                $$PATH_TO_COMMON/Workflow/GRAPHICS \
                $$PATH_TO_COMMON/Workflow/EXECUTION \
                $$PATH_TO_COMMON/Workflow/EDP \
                $$PATH_TO_COMMON/Workflow/SIM \

SOURCES +=  $$PATH_TO_COMMON/Workflow/UQ/DakotaResults.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/GaussianProcessInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/LatinHypercubeInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsSampling.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsReliability.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsSensitivity.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/ImportanceSamplingInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/MonteCarloInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/PCEInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_MethodInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputSampling.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputReliability.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputSensitivity.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/InputWidgetUQ.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Results.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Engine.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaEngine.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/LocalReliabilityWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/GlobalReliabilityWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_EngineSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/MainWindowWorkflowApp.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/WorkflowAppWidget.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/JsonConfiguredWidget.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComboBox.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterFileInput.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterRVLineEdit.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RunLocalWidget.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/AgaveCurl.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/LocalApplication.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteApplication.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteService.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteJobManager.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/UserDefinedApplication.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/Controller2D.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/GlWidget2D.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/MyTableWidget.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/GraphicView2D.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/SimCenterGraphPlot.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/qcustomplot.cpp \
            $$PATH_TO_COMMON/Workflow/EDP/StandardEarthquakeEDP.cpp \
            $$PATH_TO_COMMON/Workflow/EDP/UserDefinedEDP.cpp \
            $$PATH_TO_COMMON/Workflow/EDP/EDP.cpp \
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesBuildingModel.cpp \
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesParser.cpp \
#            $$PATH_TO_COMMON/Workflow/SIM/MDOF_BuildingModel.cpp \


HEADERS +=  $$PATH_TO_COMMON/Workflow/UQ/DakotaResults.h \
            $$PATH_TO_COMMON/Workflow/UQ/GaussianProcessInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/LatinHypercubeInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsSampling.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsReliability.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaResultsSensitivity.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputReliability.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputSensitivity.h \
            $$PATH_TO_COMMON/Workflow/UQ/ImportanceSamplingInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/MonteCarloInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/PCEInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_MethodInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaInputSampling.h \
            $$PATH_TO_COMMON/Workflow/UQ/InputWidgetUQ.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Results.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Engine.h \
            $$PATH_TO_COMMON/Workflow/UQ/DakotaEngine.h \
            $$PATH_TO_COMMON/Workflow/UQ/LocalReliabilityWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/GlobalReliabilityWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_EngineSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/MainWindowWorkflowApp.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/WorkflowAppWidget.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/JsonConfiguredWidget.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComboBox.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterFileInput.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterRVLineEdit.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RunLocalWidget.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/AgaveCurl.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/LocalApplication.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteApplication.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteService.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/RemoteJobManager.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/UserDefinedApplication.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/Controller2D.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/GlWidget2D.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/MyTableWidget.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/GraphicView2D.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/SimCenterGraphPlot.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/qcustomplot.h \
            $$PATH_TO_COMMON/Workflow/EDP/StandardEarthquakeEDP.h \
            $$PATH_TO_COMMON/Workflow/EDP/UserDefinedEDP.h \
            $$PATH_TO_COMMON/Workflow/EDP/EDP.h \
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesBuildingModel.h \
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesParser.h \
#            $$PATH_TO_COMMON/Workflow/SIM/MDOF_BuildingModel.h \

