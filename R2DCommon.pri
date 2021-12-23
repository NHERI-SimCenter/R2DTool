#--------------------------------------#
#                                      #
# Project created by Stevan Gavrilovic #
#                                      #
#  University of California, Berkeley  #
#--------------------------------------#

INCLUDEPATH +=  $$PATH_TO_COMMON/Workflow/UQ \
                $$PATH_TO_COMMON/Workflow/UQ/dakota \
                $$PATH_TO_COMMON/Workflow/ANALYSIS \
                $$PATH_TO_COMMON/Workflow/WORKFLOW \	
                $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems \
                $$PATH_TO_COMMON/Workflow/WORKFLOW/Utils \	
                $$PATH_TO_COMMON/Workflow/GRAPHICS \
                $$PATH_TO_COMMON/Workflow/EXECUTION \
                $$PATH_TO_COMMON/Workflow/EDP \
                $$PATH_TO_COMMON/Workflow/SIM \

SOURCES +=  $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResults.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/GaussianProcessInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/LatinHypercubeInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsSampling.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsReliability.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsSensitivity.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/ImportanceSamplingInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/MonteCarloInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/PCEInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_MethodInputWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputSampling.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputReliability.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputSensitivity.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Results.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Engine.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaEngine.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/LocalReliabilityWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/GlobalReliabilityWidget.cpp \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_EngineSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/MainWindowWorkflowApp.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/WorkflowAppWidget.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/JsonConfiguredWidget.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComboBox.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterDoubleSpinBox.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterSpinBox.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterLineEdit.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterFileInput.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterRVLineEdit.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/Utils/ExampleDownloader.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/Utils/NetworkDownloadManager.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/CheckableTreeModel.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/SimCenterTreeView.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeItem.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeViewStyle.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/LayerTreeModel.cpp \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/LayerTreeItem.cpp \
            $$PATH_TO_COMMON/Workflow/ANALYSIS/InputWidgetOpenSeesAnalysis.cpp \
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
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesParser.cpp
#            $$PATH_TO_COMMON/Workflow/SIM/MDOF_BuildingModel.cpp \


HEADERS +=  $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResults.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/GaussianProcessInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/LatinHypercubeInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsSampling.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsReliability.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaResultsSensitivity.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputReliability.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputSensitivity.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/ImportanceSamplingInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/MonteCarloInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/PCEInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_MethodInputWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaInputSampling.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Results.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_Engine.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/DakotaEngine.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/LocalReliabilityWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/dakota/GlobalReliabilityWidget.h \
            $$PATH_TO_COMMON/Workflow/UQ/UQ_EngineSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/MainWindowWorkflowApp.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/WorkflowAppWidget.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/JsonConfiguredWidget.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComboBox.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterDoubleSpinBox.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterSpinBox.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterLineEdit.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterFileInput.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterRVLineEdit.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/Utils/ExampleDownloader.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/Utils/NetworkDownloadManager.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/CheckableTreeModel.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/SimCenterTreeView.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeItem.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeViewStyle.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/LayerTreeModel.h \
	    $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/LayerTreeItem.h \
            $$PATH_TO_COMMON/Workflow/ANALYSIS/InputWidgetOpenSeesAnalysis.h \
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
            $$PATH_TO_COMMON/Workflow/SIM/OpenSeesParser.h