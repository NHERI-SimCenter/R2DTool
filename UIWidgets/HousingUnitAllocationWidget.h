#ifndef HousingUnitAllocationWidget_H
#define HousingUnitAllocationWidget_H
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "SimCenterAppWidget.h"

#include <qgsfeature.h>
#include <QMap>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <set>
#include <thread>
#include <mutex>

class VisualizationWidget;

class QgsVectorLayer;
class QGISVisualizationWidget;
class QgsProjectionSelectionWidget;

class QProcess;
class QPushButton;
class QLineEdit;
class QComboBox;
class QNetworkReply;
class QNetworkAccessManager;

struct Building;
struct CensusBlockGroup;

struct Parcel
{
    Parcel() {}

    QgsFeature parcelFeat;

    QgsFeatureId getFeatureId(){return parcelFeat.id();}

    std::vector<std::shared_ptr<Building>> associatedBuildings;
};


struct Building
{
    Building() {}

    QgsFeature buildingFeat;
    QgsPointXY buildingCentroidXY;

    QgsFeatureId getFeatureId(){return buildingFeat.id();}

    std::shared_ptr<Parcel> associatedParcel;
};


// Struct to store block information
struct CensusBlock
{
    CensusBlock() {}

    QgsFeature blockFeat;

    QgsFeatureId getFeatureId(){return blockFeat.id();}

    std::shared_ptr<CensusBlockGroup> associatedCensusBlockGroup;
    std::vector<std::shared_ptr<Parcel>> associatedParcels;
};


// Struct to store block group information
struct CensusBlockGroup
{
    CensusBlockGroup() {}

    QgsFeature blockGroupFeat;

    QgsFeatureId getFeatureId(){return blockGroupFeat.id();}

    std::vector<std::shared_ptr<CensusBlock>> associatedBlocks;
};


struct HousingUnit
{
    HousingUnit() {}

    QgsFeature housingUnitFeat;

    QgsFeatureId getFeatureId(){return housingUnitFeat.id();}

    std::shared_ptr<Building> associatedBuilding;
};

class HousingUnitAllocationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit HousingUnitAllocationWidget(QWidget *parent, VisualizationWidget* visWidget);
    virtual ~HousingUnitAllocationWidget();

    bool outputAppDataToJSON(QJsonObject &jsonObject) override;
    bool inputAppDataFromJSON(QJsonObject &jsonObject) override;

    int importJoinAssets();

    void clear() override;

signals:

    void emitErrorMsg(QString);
    void emitStatusMsg(QString);
    void emitInfoMsg(QString);

    void emitCreateLayerMainThread(QString,QString);

    void emitLayerCreationFinished();

    void emitStartProcessMainThread(QString,QStringList);

    void emitDownloadMainThread(QString,QString);


private slots:

    // Handle the status/error messages from the multiple threads (progress dialog must be called from the main thread)
    void handleErrorMsg(const QString msg);
    void handleStatusMsg(const QString msg);
    void handleInfoMsg(const QString msg);

    // Creates the .GDB GIS files from which the population demographics will be extracted to append to the housing database
    // This runs a python script
    int createGISFiles(void);

    // Handles the results when the user is finished
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // Brings up the dialog and tells the user that the process has started
    void handleProcessStarted(void);

    // Displays the text output of the process in the dialog
    void handleProcessTextOutput(void);

    void handleCreateGISFilesButtonPressed(void);

    void handleBlockLayerCrsChanged(const QgsCoordinateReferenceSystem & val);
    void handleBuildingLayerCrsChanged(const QgsCoordinateReferenceSystem & val);

    void browseGISresultsFolderDir(void);
    void browseBlockLevelGISFile(void);
    void browseBuildingGISFile(void);

    int importBuidlingsLayer(void);
    int importCensusBlockLayer(void);

    int handleRunJoinButtonPressed();

    void handleCreateLayerMainThread(const QString& path, const QString& name);
    void handleProcessStartedMainThread(const QString& pythonPath, const QStringList& args);


    void handleDownloadMainThread(const QString& url, const QString& path);

    void handleDownloadFinished(QNetworkReply* reply);

    void handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void handleDownloadError(QNetworkReply::NetworkError code);

private:

    QWidget* getHUAWidget(void);

    // Returns a set of counties that the building inventory resides in
    std::set<QString> getCountiesFromBuildingInventory(void);

    std::unique_ptr<std::thread> thread;

    QgsVectorLayer* parcelsLayer = nullptr;

    void getStandardOutputDir(void);

    // Mutex for the building layer
    std::mutex buildingLayerSemaphore;

    QgsVectorLayer* buildingsLayer = nullptr;
    QgsVectorLayer* addressPointLayer = nullptr;
    QgsVectorLayer* blockLayer = nullptr;
    QgsVectorLayer* blockGroupLayer = nullptr;

    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    // Store the assets in a hash map for quick access according to its ID
    QMap<QgsFeatureId,std::shared_ptr<Building>> buildingsMap;
    QMap<QgsFeatureId,std::shared_ptr<Parcel>> parcelsMap;
    QMap<QgsFeatureId,std::shared_ptr<HousingUnit>> housingUnitMap;

    int getBuildingFeatures(void);
    int getParcelFeatures(void);

    int linkBuildingsAndParcels(void);

    int extractCensusData(void);

    QProcess* process = nullptr;

    QgsProjectionSelectionWidget* mblockLevelCrsSelector = nullptr;

    QgsProjectionSelectionWidget* buildingCrsSelector = nullptr;


    QComboBox* censusVintageCombo = nullptr;
    QLineEdit* blockLevelPathLineEdit = nullptr;
    QLineEdit* gisDirLineEdit = nullptr;

    QLineEdit* buildingsPathLineEdit = nullptr;

    QPushButton* runGetCensusButton = nullptr;

    QPushButton* runJoinButton = nullptr;

    // Map to store layers that were created in the main thread
    QMap<QString, QgsVectorLayer*> Layermap;

    bool downloadCountyFiles(const QString& path);

    bool downloadingCensus = false;

    QNetworkAccessManager fileDownloadManager;


};

#endif // HousingUnitAllocationWidget_H
