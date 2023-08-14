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

#include "HousingUnitAllocationWidget.h"
#include "SimCenterPreferences.h"
#include "AssetInputDelegate.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"
#include "QGISVisualizationWidget.h"
#include <Utils/ProgramOutputDialog.h>
#include "NetworkDownloadManager.h"
#include "ZipUtils.h"

#include <QDir>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>
#include <QHeaderView>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QButtonGroup>
#include <QRadioButton>
#include <QStackedWidget>

#include <qgsprojectionselectionwidget.h>
#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsgeometryengine.h>
#include <qgsproject.h>
#include <qgsmapcanvas.h>

// Test to remove start
#include <chrono>
#include <thread>
#include <future>
using namespace std::chrono;
// Test to remove end


HousingUnitAllocationWidget::HousingUnitAllocationWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget=static_cast<QGISVisualizationWidget*>(visWidget);

    this->setContentsMargins(0,0,0,0);


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(this->getHUAWidget());
    layout->setSpacing(0);
    layout->addStretch();

    // Create the process where python script will execute
    process = new QProcess(this);

    // Connect signals and slots for multithreaded execution
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &HousingUnitAllocationWidget::handleProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput, this, &HousingUnitAllocationWidget::handleProcessTextOutput);
    connect(process, &QProcess::started, this, &HousingUnitAllocationWidget::handleProcessStarted);

    connect(this,&HousingUnitAllocationWidget::emitStatusMsg,this,&HousingUnitAllocationWidget::handleStatusMsg);
    connect(this,&HousingUnitAllocationWidget::emitErrorMsg,this,&HousingUnitAllocationWidget::handleErrorMsg);
    connect(this,&HousingUnitAllocationWidget::emitInfoMsg,this,&HousingUnitAllocationWidget::handleInfoMsg);

    connect(this,&HousingUnitAllocationWidget::emitCreateLayerMainThread,this,&HousingUnitAllocationWidget::handleCreateLayerMainThread);
    connect(this,&HousingUnitAllocationWidget::emitStartProcessMainThread,this,&HousingUnitAllocationWidget::handleProcessStartedMainThread);

    connect(&fileDownloadManager,&QNetworkAccessManager::finished, this, &HousingUnitAllocationWidget::handleDownloadFinished);

    connect(this,&HousingUnitAllocationWidget::emitDownloadMainThread,this,&HousingUnitAllocationWidget::handleDownloadMainThread);

    // Test to remove start
    //    auto buildingsGISFile =  "/Users/steve/Desktop/SimCenter/Examples/SanFranciscoTestbed/SanFranciscoBuildingFootprints/SanFrancisco_buildingfootprints_2014.shp";

    //    buildingsPathLineEdit->setText(buildingsGISFile);
    //    this->importBuidlingsLayer();
    //    buildingCrsSelector->setCrs(QgsCoordinateReferenceSystem("ESRI:102643"));

    //    censusVintageCombo->setCurrentText("2010");
    //    ACSVintageCombo->setCurrentText("2010");

    // this->createGISFiles();
    // Test to remove end

    // get current layers
    this->handleGetLayersFromMap();
}


HousingUnitAllocationWidget::~HousingUnitAllocationWidget()
{

}


std::set<QString> HousingUnitAllocationWidget::getCountiesFromBuildingInventory(void)
{

    std::set<QString> res;

    if(assetLayer == nullptr)
    {
        emit emitErrorMsg("The asset layer does not exist, please import the assets");
        return res;
    }


    auto pathToCountiesGIS = QCoreApplication::applicationDirPath() +
            QDir::separator() + "Databases" + QDir::separator() + "USCounties2021"+ QDir::separator() + "tl_2021_us_county.shp";

    QFile file(pathToCountiesGIS);

    // Return if database does not exist
    if(!file.exists())
    {

        if(downloadingCensus)
            return res;

        emit emitErrorMsg("The counties shapefile does not exist. Attempting to download the county files");

        auto countiesFolder =  pathToCountiesGIS = QCoreApplication::applicationDirPath() +
                QDir::separator() + "Databases" + QDir::separator() + "USCounties2021";

        QDir dirWork(countiesFolder);

        if (!dirWork.exists())
            if (!dirWork.mkpath(countiesFolder))
            {
                QString errorMessage = QString("Could not create the directory: ") + countiesFolder;

                emit emitErrorMsg(errorMessage);

                return res;
            }

        auto savePath =countiesFolder + QDir::separator() + "counties.zip";

        downloadCountyFiles(savePath);

        return res;
    }

    emit emitStatusMsg("Getting counties containing the asset inventory.");

    //auto countiesLayer = theVisualizationWidget->addVectorLayer(pathToCountiesGIS, "Counties", "ogr");

    QEventLoop pause;

    connect(this, SIGNAL(emitLayerCreationFinished()), &pause, SLOT(quit()));

    emit emitCreateLayerMainThread(pathToCountiesGIS,"Counties");

    pause.exec();

    QgsVectorLayer* countiesLayer  = Layermap.value("Counties",nullptr);

    if(countiesLayer == nullptr)
    {
        emit emitErrorMsg("Could not create the counties layer");
        return res;
    }

    // If layer found then remove from map
    Layermap.remove("Counties");

    //countiesLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:9001"));
    countiesLayer->setOpacity(0.50);

    // First get the counties features because there may be less of them
    QVector<QgsFeature> countyFeatVec;

    countyFeatVec.reserve(countiesLayer->featureCount());

    // Iterate through the asset features
    auto countyFeatures = countiesLayer->getFeatures();

    QgsFeature countyFeat;

    // Coordinate transformation to transform from the county layer crs to the building layer crs - in the case where they are different
    if(assetLayer->crs() != countiesLayer->crs())
    {
        QgsCoordinateTransform coordTrans(countiesLayer->crs(), assetLayer->crs(), QgsProject::instance());
        while (countyFeatures.nextFeature(countyFeat))
        {
            auto geom = countyFeat.geometry();
            geom.get()->transform(coordTrans);
            countyFeat.setGeometry(geom);
            countyFeatVec.push_back(countyFeat);
        }
    }
    else
    {
        while (countyFeatures.nextFeature(countyFeat))
            countyFeatVec.push_back(countyFeat);
    }


    // Create a cache to store the counties, since many buildings can be in multiple counties
    QVector<QgsFeature> countyVecCache;


    // Iterate through the building features
    auto features = assetLayer->getFeatures();

    auto testInPolygon = [](const QgsGeometry& countyGeom, const QgsPointXY& buildCentroid, QgsGeometryEngine* polygonGeometryEngine) -> bool
    {

        // Do initial bounding box check which is very fast to exclude points that are far away
        auto bb = countyGeom.boundingBox();

        if(bb.contains(buildCentroid))
        {
            if(polygonGeometryEngine->intersects(countyGeom.constGet()))
            {
                return true;
            }
        }

        return false;
    };


    QgsFeature feat;
    while (features.nextFeature(feat))
    {

        auto buildGeom = feat.geometry();

        auto buildCentroid = buildGeom.centroid().asPoint();

        std::unique_ptr< QgsGeometryEngine > polygonGeometryEngine(QgsGeometry::createGeometryEngine(buildGeom.constGet()));

        polygonGeometryEngine->prepareGeometry();

        //        if(!polygonGeometryEngine->isValid())
        //        {
        //            emit emitStatusMsg("Error: the geometry engine is not valid for feature "+QString::number(feat.id()));
        //            return std::set<QString>{};
        //        }

        bool found = false;

        // First check the cache
        auto countyIt = countyVecCache.constBegin();
        while(countyIt != countyVecCache.constEnd())
        {
            auto countyGeom = countyIt->geometry();

            if(testInPolygon(countyGeom,buildCentroid,polygonGeometryEngine.get()) == true)
            {

                auto countyIDidx = countyIt->fieldNameIndex("GEOID");
                auto countyId = countyIt->attribute(countyIDidx).toString();

                res.insert(countyId);

                found = true;
                break;
            }

            countyIt++;
        }

        // Try to find it in the full county vector
        if(!found)
        {
            auto countyIt = countyFeatVec.constBegin();
            while(countyIt != countyFeatVec.constEnd())
            {
                auto countyGeom = countyIt->geometry();

                if(testInPolygon(countyGeom,buildCentroid,polygonGeometryEngine.get()) == true)
                {

                    auto countyIDidx = countyIt->fieldNameIndex("GEOID");
                    auto countyId = countyIt->attribute(countyIDidx).toString();

                    res.insert(countyId);

                    found = true;
                    break;
                }

                countyIt++;
            }
        }

        // If still not found then error
        if(!found)
        {
            emit emitErrorMsg("Error, could not find a US county for the feature" + QString::number(feat.id()));

            return std::set<QString>{};
        }

    }

    emit emitStatusMsg("Done getting counties for the asset inventory.");

    return res;
}


void HousingUnitAllocationWidget::handleCreateGISFilesButtonPressed(void)
{
    thread.release();
    thread = std::make_unique<std::thread>(&HousingUnitAllocationWidget::createGISFiles, this);
}


int HousingUnitAllocationWidget::createGISFiles(void)
{
    if(assetLayer == nullptr)
    {
        emit emitErrorMsg("The asset layer does not exist, please import assets");

        return -1;
    }

    auto outputDir = gisDirLineEdit->text();

    if(outputDir.isEmpty())
    {
        emit emitErrorMsg("Output directory is empty. Set a directory where the GIS files will be saved");

        return -1;
    }

    QString pathToResultsDirectory = outputDir + QDir::separator();

    QDir dirRes(pathToResultsDirectory);

    if (!dirRes.exists())
    {
        if (!dirRes.mkpath(pathToResultsDirectory))
        {
            QString errorMessage = QString("Could not create Directory: ") + pathToResultsDirectory
                    + QString(". Check the directory.");

            emit emitErrorMsg(errorMessage);

            return -1;
        }
    }
    else
    {
        // Get the existing files in the folder to see if we already have the record
        QStringList acceptableFileExtensions = {"*.json","*.geojson","*.shp","*.cpg","*.dbf","*.prj","*.shx"};
        QStringList existingFiles = dirRes.entryList(acceptableFileExtensions, QDir::Files);

        // Remove the csv files - in case we have less sites that previous and they are not overwritten
        for(auto&& it : existingFiles)
        {
            QFile file(pathToResultsDirectory + it);
            file.remove();
        }
    }


    // First get the counties from the building inventory
    auto countiesSet = getCountiesFromBuildingInventory();

    if(countiesSet.empty())
    {
        if(downloadingCensus == true)
        {
            QString statusMessage = "Download of census county files in progress please wait. It might take a while.";
            emit emitStatusMsg(statusMessage);
            return 0;
        }

        QString errorMessage = "Empty set of counties, check buildings exist and coordinate reference system places them within the US.";
        emit emitErrorMsg(errorMessage);
        return -1;
    }

    QJsonArray countiesArr;

    for(auto&& it: countiesSet)
    {
        countiesArr.append(it);
    }

    auto censusVintage = censusVintageCombo->currentText();
    auto ACSVintage = ACSVintageCombo->currentText();

    QJsonArray censusVarsArr;
    auto censusVarsText = censusVarsLineEdit->text();

    if(!censusVarsText.isEmpty())
    {
        QStringList censusVars =censusVarsText.split( ",",Qt::SkipEmptyParts);

        for(auto&& it : censusVars)
            censusVarsArr.append(it);
    }

    QJsonArray ACSVarsArr;
    auto ACSVarsText = ACSVarsLineEdit->text();

    if(!ACSVarsText.isEmpty())
    {

        QStringList ACSVars = ACSVarsText.split( ",", Qt::SkipEmptyParts);


        for(auto&& it : ACSVars)
            ACSVarsArr.append(it);
    }

    QJsonObject configFile;

    configFile.insert("OutputDirectory",outputDir);
    configFile.insert("CountiesArray",countiesArr);
    configFile.insert("PopulationDemographicsVintage",censusVintage);
    configFile.insert("HouseholdIncomeVintage",ACSVintage);

    configFile.insert("CensusVariablesArray",censusVarsArr);
    configFile.insert("ACSVariablesArray",ACSVarsArr);

    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);

    // Output the config file
    QString pathToConfigFile = outputDir + QDir::separator() + "HUAConfig.json";

    QFile file(pathToConfigFile);

    if(!file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }

    // Get the python path
    auto pythonPath = SimCenterPreferences::getInstance()->getPython();


    QString pathToHUAScript = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
            + "applications" + QDir::separator() + "performHUA" + QDir::separator() + "INCORECensusUtil.py";

    QFileInfo hazardFileInfo(pathToHUAScript);
    if (!hazardFileInfo.exists()) {
        QString errorMessage = QString("ERROR - INCORE census util does not exist") + pathToHUAScript;
        emit emitErrorMsg(errorMessage);
        return -1;
    }
    QStringList args = {pathToHUAScript,"--census_config",pathToConfigFile};

    qDebug()<<"Census Python Path: "<<pathToHUAScript;
    qDebug()<<"Census Command: "<<args[0]<<" "<<args[1]<<" "<<args[2];

    emit emitStartProcessMainThread(pythonPath, args);

    return 0;
}


int HousingUnitAllocationWidget::handleRunJoinButtonPressed()
{
    //    auto addressPointGISFile =  "/Users/steve/Desktop/AnchorageTestbed/Addresses_Hosted/Address_Point.shp";

    //    addressPointLayer = theVisualizationWidget->addVectorLayer(addressPointGISFile, "Address Points", "ogr");

    //    auto censusBlocksGISFile = "/Users/steve/Desktop/AnchorageTestbed/AlaskaCensus/AkBlocks2020/Blocks2020.gpkg";


    //    auto censusBlockGroupsGISFile = "/Users/steve/Desktop/AnchorageTestbed/AlaskaCensus/ALASKA_INCOME_BG/AKBGIncome2019.gpkg";

    //    blockGroupLayer = theVisualizationWidget->addVectorLayer(censusBlockGroupsGISFile, "Census Block Groups", "ogr");
    //    blockGroupLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    //    blockGroupLayer->setOpacity(0.50);

    //    auto parcelGISFile =  "/Users/steve/Desktop/AnchorageTestbed/AnchorageTaxParcels/AnchorageTaxParcels.gpkg";
    //    parcelsLayer = theVisualizationWidget->addVectorLayer(parcelGISFile, "Tax Parcels", "ogr");
    //    parcelsLayer->setOpacity(0.50);
    //    parcelsLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));


    this->statusMessage("Starting extraction of census data,  linking buildings to parcels,  and addresses to parcels.  Process running in the background and it may take a while.");
    QApplication::processEvents();

    thread.release();
    thread = std::make_unique<std::thread>(&HousingUnitAllocationWidget::importJoinAssets, this);

    return 0;
}


int HousingUnitAllocationWidget::importBuidlingsLayer(void)
{
    this->statusMessage("Importing asset layer.");

    auto path = buildingsPathLineEdit->text();

    auto origBuildingLayer = theVisualizationWidget->addVectorLayer(path, "Assets", "ogr");

    if(origBuildingLayer == nullptr)
    {
        this->errorMessage("Error importing the asset layer");
        return -1;
    }

    origBuildingLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    // Create a duplicate layer since we will be modifying it
    assetLayer = theVisualizationWidget->duplicateExistingLayer(origBuildingLayer);

    if(assetLayer == nullptr)
    {
        this->errorMessage("Error copying the asset layer");
        return -1;
    }

    this->statusMessage("Importing asset layer complete.");

    return 0;
}


int HousingUnitAllocationWidget::importCensusDemographicsLayer(void)
{

    this->statusMessage("Importing census population demographics layer.");

    auto path = censusPathLineEdit->text();

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        this->errorMessage("Error the file does not exist at the path: "+path);
        return -1;
    }


    // Create the census block layer
    censusBlockLayer = theVisualizationWidget->addVectorLayer(path,censusVintageCombo->currentText() + "Census Population Demographics", "ogr");

    if(censusBlockLayer == nullptr)
    {
        this->errorMessage("Error creating the census block layer");
        return -1;
    }

    mCensusCrsSelector->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));
    censusBlockLayer->setOpacity(0.50);

    this->statusMessage("Importing census population demographics layer complete.");

    return 0;
}


int HousingUnitAllocationWidget::importACSIncomeLayer(void)
{

    this->statusMessage("Importing ACS household income layer.");

    auto path = ACSPathLineEdit->text();

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        this->errorMessage("Error the file does not exist at the path: "+path);
        return -1;
    }


    // Create the census block layer
    ACSBlockGroupLayer = theVisualizationWidget->addVectorLayer(path,ACSVintageCombo->currentText()+" ACS Household Income", "ogr");

    if(ACSBlockGroupLayer == nullptr)
    {
        this->errorMessage("Error creating the census block layer");
        return -1;
    }

    mACSCrsSelector->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));
    ACSBlockGroupLayer->setOpacity(0.50);

    this->statusMessage("Importing ACS household income layer complete.");

    return 0;
}


int HousingUnitAllocationWidget::importJoinAssets()
{
    emit emitStatusMsg("Extracting informatiom from census layer(s) to add to assets.");

    auto future1 = std::async(&HousingUnitAllocationWidget::extractCensusData, this);

    if(future1.get() != 0)
    {
        emit emitErrorMsg("Error extracting census data");
        return -1;
    }

    auto future2 = std::async(&HousingUnitAllocationWidget::extractACSData, this);

    if(future2.get() != 0)
    {
        emit emitErrorMsg("Error extracting ACS data");
        return -1;
    }

    auto future3 = std::async(&HousingUnitAllocationWidget::getBuildingFeatures, this);

    if(future3.get() != 0)
    {
        emit emitErrorMsg("Error getting the asset features");
        return -1;
    }

    // TODO: implement parcels
    //    if(parcelsLayer == nullptr)
    //    {
    //        emit emitStatusMsg("No parcels layer found, skipping import of parcels.");
    //    }
    //    else
    //    {

    //        auto future3 = std::async(&HousingUnitAllocationWidget::getParcelFeatures, this);

    //        if(future3.get() != 0)
    //        {
    //            emit emitErrorMsg("Error getting the parcel features");
    //            return -1;
    //        }

    //        auto future4 = std::async(&HousingUnitAllocationWidget::linkBuildingsAndParcels, this);

    //        if(future4.get() != 0)
    //        {
    //            emit emitErrorMsg("Error getting the parcel features");
    //            return -1;
    //        }

    //    }

    emit emitStatusMsg("Done extracting informatiom from census layer(s).");
    QApplication::processEvents();

    return 0;
}


bool HousingUnitAllocationWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    Q_UNUSED(jsonObject);

    return true;
}


bool HousingUnitAllocationWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    Q_UNUSED(jsonObject);

    return true;
}


int HousingUnitAllocationWidget::getBuildingFeatures(void)
{
    emit emitStatusMsg("Parsing assets.");

    buildingLayerSemaphore.lock();

    if(assetLayer == nullptr)
    {
        emit emitErrorMsg("Error: no assets layer");
        buildingLayerSemaphore.unlock();

        return -1;
    }

    auto features = assetLayer->getFeatures();

    //buildingsMap.reserve(buildingsLayer->featureCount());

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        auto newBuilding = std::make_shared<Building>();

        // Create a deep copy
        newBuilding->buildingFeat = QgsFeature(feat);

        auto buildGeom = feat.geometry().get()->clone();

        auto buildCentroid = buildGeom->centroid();

        newBuilding->buildingCentroidXY = buildCentroid;

        auto featId = feat.id();
        buildingsMap.insert(featId,newBuilding);

        // buildingsMap.push_back(newBuilding);
    }

    emit emitStatusMsg("Loaded "+QString::number(assetLayer->featureCount())+" assets");

    buildingLayerSemaphore.unlock();

    return 0;
}


int HousingUnitAllocationWidget::getParcelFeatures(void)
{
    emit emitStatusMsg("Parsing parcels.");

    if(parcelsLayer == nullptr)
    {
        emit emitErrorMsg("Error: no parcels layer");
        return -1;
    }

    // Coordinate transform to transform parcel layer into the building layers coordinate system
    buildingLayerSemaphore.lock();
    QgsCoordinateTransform ct(parcelsLayer->crs(), assetLayer->crs(), QgsProject::instance());
    buildingLayerSemaphore.unlock();

    auto features = parcelsLayer->getFeatures();

    //    parcelsMap.reserve(parcelsLayer->featureCount());

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        auto newParcel = std::make_shared<Parcel>();

        auto geom = feat.geometry();
        geom.get()->transform(ct);
        feat.setGeometry(QgsGeometry(geom.get()->clone()));

        newParcel->parcelFeat = QgsFeature(feat);

        auto featId = feat.id();
        parcelsMap.insert(featId,newParcel);

        // parcelsMap.push_back(newParcel);
    }

    emit emitStatusMsg("Loaded "+QString::number(parcelsLayer->featureCount())+" parcels");


    return 0;
}


int HousingUnitAllocationWidget::linkBuildingsAndParcels(void)
{

    // Test to remove start
    auto start = high_resolution_clock::now();
    // Test to remove end


    //auto buildingsMapCpy = buildingsMap;
    //auto pacrcelsMapCpy = buildingsMap;

    emit emitStatusMsg("Linking buildings to parcels.");

    if(buildingsMap.isEmpty() || parcelsMap.isEmpty())
    {
        emit emitErrorMsg("Error: empty buildings or parcels vector");
        return -1;
    }

    auto countFound = 0;
    auto countNotFound = 0;

    //    QMapIterator<QgsFeatureId,std::shared_ptr<Building>> buildingIt(buildingsMap);
    //    while (buildingIt.hasNext())

    for(auto&& buildObj : buildingsMap)
    {
        //        if(count == 1000)
        //            break;

        // buildingIt.next();
        //        std::shared_ptr<Building> buildObj = buildingIt.value();

        auto buildCentroidXY = buildObj->buildingCentroidXY;

        // auto buildGeom = buildObj->buildingFeat.geometry();
        //        std::unique_ptr< QgsGeometryEngine > polygonGeometryEngine(QgsGeometry::createGeometryEngine(buildGeom.constGet()));

        //        polygonGeometryEngine->prepareGeometry();

        //        if(!polygonGeometryEngine->isValid())
        //        {
        //            emit emitStatusMsg("Error: the geometry engine is not valid for feature "+QString::number(buildingIt.key()));
        //            return -1;
        //        }

        bool found = false;

        for(auto&& parcel : parcelsMap)
        {
            auto parcelGeom = parcel->parcelFeat.geometry();

            // Do initial bounding box check which is very fast to exclude points that are far away
            auto bb = parcelGeom.boundingBox();

            if(bb.contains(buildCentroidXY))
            {
                //                if(polygonGeometryEngine->intersects(parcelGeom.constGet()))
                //                {
                // Associate the parcel with the building and vice versa
                parcel->associatedBuildings.push_back(buildObj);
                buildObj->associatedParcel = parcel;
                found = true;
                ++countFound;
                break;
                //                }
            }
        }

        if(!found)
        {
            emit emitInfoMsg("Warning: could not find a parcel for building "+QString::number(buildObj->buildingFeat.id()));
            ++countNotFound;
            // return -1;
        }
    }

    emit emitStatusMsg("Done linking buildings to parcels.");

    // Test to remove start
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    emit emitStatusMsg("Duration linking buildings to parcels: " + QString::number(duration.count()/1000.0) + " seconds");
    // Test to remove end


    return 0;
}


int HousingUnitAllocationWidget::extractCensusData(void)
{
    emit emitStatusMsg("Extracting census population demographics data.");

    // Lock access to the building layer
    std::unique_lock<std::mutex> lck (buildingLayerSemaphore);

    if(censusBlockLayer == nullptr || assetLayer == nullptr)
    {
        emit emitErrorMsg("Error in extracting census data. Either a census layer or buildings layer is missing.");
        return -1;
    }

    QString errMsg;

    auto res = theVisualizationWidget->joinLayers(censusBlockLayer,assetLayer,"CENSUSLAYER_",errMsg);
    if(res != 0)
        emit emitErrorMsg(errMsg);

    emit emitStatusMsg("Done extracting census data.");

    return 0;
}


int HousingUnitAllocationWidget::extractACSData(void)
{
    emit emitStatusMsg("Extracting ACS household income data.");

    // Lock access to the building layer
    std::unique_lock<std::mutex> lck (buildingLayerSemaphore);

    if(ACSBlockGroupLayer == nullptr || assetLayer == nullptr)
    {
        emit emitErrorMsg("Error in extracting ACS data. Either a ACS layer or assets layer is missing.");
        return -1;
    }

    QString errMsg;

    auto res = theVisualizationWidget->joinLayers(ACSBlockGroupLayer,assetLayer,"ACSLAYER_",errMsg);
    if(res != 0)
        emit emitErrorMsg(errMsg);

    emit emitStatusMsg("Done extracting ACS data.");

    return 0;
}


void HousingUnitAllocationWidget::clear()
{
    Layermap.clear();
    layerNameCombo->clear();
    selectedLayerNameLabel->setText("None Selected");
}


void HousingUnitAllocationWidget::handleErrorMsg(const QString msg)
{
    this->errorMessage(msg);
}


void HousingUnitAllocationWidget::handleStatusMsg(const QString msg)
{
    this->statusMessage(msg);
}


void HousingUnitAllocationWidget::handleInfoMsg(const QString msg)
{
    this->infoMessage(msg);
}


QWidget* HousingUnitAllocationWidget::getHUAWidget(void)
{

    // file  input
    auto mainWidget = new QWidget();
    QGridLayout* mainLayout = new QGridLayout(mainWidget);
    mainWidget->setLayout(mainLayout);

    QLabel* buildingPathText = new QLabel("Path to asset GIS file (.gdb, .shp, etc.)");
    buildingsPathLineEdit = new QLineEdit();
    QPushButton *browseBuidlingFileButton = new QPushButton("Browse");

    connect(browseBuidlingFileButton,SIGNAL(clicked()),this,SLOT(browseBuildingGISFile()));

    QLabel* buildCrsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    buildingCrsSelector = new QgsProjectionSelectionWidget();
    buildingCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    buildingCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    buildingCrsSelector->setFocusPolicy(Qt::StrongFocus);

    connect(buildingCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,&HousingUnitAllocationWidget::handleBuildingLayerCrsChanged);

    auto assetPathInput = new QWidget();

    auto buildingHBoxLayout = new QHBoxLayout(assetPathInput);
    buildingHBoxLayout->addWidget(buildingPathText);
    buildingHBoxLayout->addWidget(buildingsPathLineEdit);
    buildingHBoxLayout->addWidget(browseBuidlingFileButton);
    buildingHBoxLayout->addWidget(buildCrsTypeLabel);
    buildingHBoxLayout->addWidget(buildingCrsSelector);

    // Create button group
    fromMapRadioButton = new QRadioButton(tr("Select From GIS Map"));
    fromPathButton = new QRadioButton(tr("Provide File Path"));

    //First we need to add type radio buttons
    m_typeButtonsGroup = new QButtonGroup();

    // Set the from map by default
    fromMapRadioButton->setChecked(true);

    m_typeButtonsGroup->addButton(fromMapRadioButton, 0);
    m_typeButtonsGroup->addButton(fromPathButton, 1);

    m_typeButtonsGroup->setExclusive(true);

    // Layout to select the method of asset input
    auto assetInputSelectionLayout = new QHBoxLayout();

    assetInputSelectionLayout->addWidget(fromMapRadioButton);
    assetInputSelectionLayout->addWidget(fromPathButton);
    assetInputSelectionLayout->addStretch(0);

    auto assetFromMapInput = new QWidget();

    auto selectLayerLabel = new QLabel("Select a layer from the GIS map");
    layerNameCombo = new QComboBox();
    layerNameCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    auto layerSelectedLabel = new QLabel("Current Selected Layer:");

    selectedLayerNameLabel = new QLabel("None Selected");

    connect(layerNameCombo,&QComboBox::currentTextChanged,this,&HousingUnitAllocationWidget::handleSelectLayerFromMap);

    // Update the combobox whenever layers on the map change
    auto canvas = theVisualizationWidget->getMainCanvas();
    connect(canvas,&QgsMapCanvas::layersChanged,this,&HousingUnitAllocationWidget::handleGetLayersFromMap);

    auto assetFromMapLayout = new QHBoxLayout(assetFromMapInput);
    assetFromMapLayout->addWidget(selectLayerLabel);
    assetFromMapLayout->addWidget(layerNameCombo);
    assetFromMapLayout->addWidget(layerSelectedLabel);
    assetFromMapLayout->addWidget(selectedLayerNameLabel);
    assetFromMapLayout->addStretch(0);

    // Create the stacked widget that will change depending on the selected way to input an asset
    auto inputStackedWidget = new QStackedWidget();

    inputStackedWidget->addWidget(assetFromMapInput);
    inputStackedWidget->addWidget(assetPathInput);

    connect(m_typeButtonsGroup, QOverload<int>::of(&QButtonGroup::idReleased), [=](int id)
    {
        if(id == 0)
            inputStackedWidget->setCurrentIndex(0);
        else if (id == 1)
            inputStackedWidget->setCurrentIndex(1);
    });

    // The main layout containing the selection buttons and the stacked widget
    auto assetInputLayout = new QVBoxLayout();

    assetInputLayout->addLayout(assetInputSelectionLayout);
    assetInputLayout->addWidget(inputStackedWidget);

    QGroupBox* censusDataGB = new QGroupBox("Download Census & ACS Data (create .gdb files from scratch)");
    QGridLayout* cdGBLayout = new QGridLayout(censusDataGB);

    QLabel* censusVintageLabel = new QLabel("Decennial Census date:");
    censusVintageCombo = new QComboBox();
    censusVintageCombo->addItems(QStringList({"2010","2020"}));
    censusVintageCombo->setCurrentText("2010");

    QLabel* ACSVintageLabel = new QLabel("American Community Survey date:");
    ACSVintageCombo = new QComboBox();
    ACSVintageCombo->addItems(QStringList({"2010","2015","2020"}));
    ACSVintageCombo->setCurrentText("2010");


    // Output dir where the results will be stored
    QLabel* gisDirLabel = new QLabel(tr("Directory to store created .GIS files"),this);
    gisDirLineEdit = new QLineEdit();

    // Populate the standard output dir
    this->getStandardOutputDir();

    auto getGISFolderButton = new QPushButton(tr("Browse"),this);
    connect(getGISFolderButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::browseGISresultsFolderDir);

    runGetCensusButton = new QPushButton("Download Census Data");
    connect(runGetCensusButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::handleCreateGISFilesButtonPressed);

    // Line edit to add custom census variables to pull from API
    censusVarsLineEdit = new QLineEdit;
    censusVarsLineEdit->setPlaceholderText("Enter custom census variables as a comma separated list, e.g., P2_001N,P2_002N,P2_005N");

    QCheckBox* customCensusVarsCheckBox = new QCheckBox;
    customCensusVarsCheckBox->setText("Custom census variables");
    customCensusVarsCheckBox->setChecked(false);

    censusVarsLineEdit->setEnabled(false);

    connect(customCensusVarsCheckBox,&QCheckBox::toggled,this,[&](bool state){
        censusVarsLineEdit->setEnabled(state);
        if(!state)
            censusVarsLineEdit->clear();});

    // Line edit to add custom census variables to pull from API
    ACSVarsLineEdit = new QLineEdit;
    ACSVarsLineEdit->setPlaceholderText("Enter custom ACS variables as a comma separated list, e.g., B19001_001E,B19001_002E");

    QCheckBox* customACSVarsCheckBox = new QCheckBox;
    customACSVarsCheckBox->setText("Custom ACS variables");
    customACSVarsCheckBox->setChecked(false);

    ACSVarsLineEdit->setEnabled(false);

    connect(customACSVarsCheckBox,&QCheckBox::toggled,this,[&](bool state){
        ACSVarsLineEdit->setEnabled(state);
        if(!state)
            ACSVarsLineEdit->clear();});


    cdGBLayout->addWidget(censusVintageLabel,0,0);
    cdGBLayout->addWidget(censusVintageCombo,0,1,1,2);

    cdGBLayout->addWidget(customCensusVarsCheckBox,1,0);
    cdGBLayout->addWidget(censusVarsLineEdit,1,1,1,2);

    cdGBLayout->addWidget(ACSVintageLabel,2,0);
    cdGBLayout->addWidget(ACSVintageCombo,2,1,1,2);

    cdGBLayout->addWidget(customACSVarsCheckBox,3,0);
    cdGBLayout->addWidget(ACSVarsLineEdit,3,1,1,2);

    cdGBLayout->addWidget(gisDirLabel,4,0);
    cdGBLayout->addWidget(gisDirLineEdit,4,1);
    cdGBLayout->addWidget(getGISFolderButton,4,2);

    cdGBLayout->addWidget(runGetCensusButton,5,0,1,3);

    QLabel* selectCensusPathText = new QLabel("Decennial Census Data GIS file (.gdb, .shp, etc.) for population demographics");
    censusPathLineEdit = new QLineEdit();
    QPushButton *browseCensusFileButton = new QPushButton("Browse");
    connect(browseCensusFileButton,SIGNAL(clicked()),this,SLOT(browseCensusGISFile()));

    QLabel* censusCrsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    mCensusCrsSelector = new QgsProjectionSelectionWidget();
    mCensusCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    mCensusCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    mCensusCrsSelector->setFocusPolicy(Qt::StrongFocus);
    connect(mCensusCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,&HousingUnitAllocationWidget::handleCensusLayerCrsChanged);

    QGroupBox* censusGroupBox = new QGroupBox("Decennial Census");
    censusGroupBox->setContentsMargins(0,0,0,0);
    QGridLayout* censusLayout = new QGridLayout(censusGroupBox);

    censusLayout->addWidget(selectCensusPathText, 1,0);
    censusLayout->addWidget(censusPathLineEdit, 1,1);
    censusLayout->addWidget(browseCensusFileButton, 1,2);
    censusLayout->addWidget(censusCrsTypeLabel,2,0);
    censusLayout->addWidget(mCensusCrsSelector,2,1);


    QLabel* selectACSPathText = new QLabel("ACS Data GIS file (.gdb, .shp, etc.) for household income and employment demographics");
    ACSPathLineEdit = new QLineEdit();
    QPushButton *browseACSFileButton = new QPushButton("Browse");
    connect(browseACSFileButton,SIGNAL(clicked()),this,SLOT(browseACSGISFile()));

    QLabel* ACScrsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    mACSCrsSelector = new QgsProjectionSelectionWidget();
    mACSCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    mACSCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    mACSCrsSelector->setFocusPolicy(Qt::StrongFocus);
    connect(mACSCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,&HousingUnitAllocationWidget::handleACSLayerCrsChanged);

    QGroupBox* ACSGroupBox = new QGroupBox("American Community Survey (ACS)");
    ACSGroupBox->setContentsMargins(0,0,0,0);
    QGridLayout* ACSLayout = new QGridLayout(ACSGroupBox);

    ACSLayout->addWidget(selectACSPathText, 1,0);
    ACSLayout->addWidget(ACSPathLineEdit, 1,1);
    ACSLayout->addWidget(browseACSFileButton, 1,2);
    ACSLayout->addWidget(ACScrsTypeLabel,2,0);
    ACSLayout->addWidget(mACSCrsSelector,2,1);

    mainLayout->addLayout(assetInputLayout,0,0,1,5);
    mainLayout->addWidget(censusDataGB, 1,0,1,5);
    mainLayout->addWidget(censusGroupBox, 2,0,1,5);
    mainLayout->addWidget(ACSGroupBox, 3,0,1,5);

    // Button to run the join between all layers
    runJoinButton = new QPushButton("Run Join");
    connect(runJoinButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::handleRunJoinButtonPressed);

    mainLayout->addWidget(runJoinButton,4,0);
    mainLayout->setRowStretch(4,1);

    return mainWidget;
}


void HousingUnitAllocationWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    this->runGetCensusButton->setEnabled(true);

    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the process to get GIS data from census failed");
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in getting GIS data from census, the exit code is " + QString::number(exitCode));
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }


    this->getProgressDialog()->hideProgressBar();

    QApplication::processEvents();

    auto pathToResultsDirectory = gisDirLineEdit->text() + QDir::separator();

    QDir dirRes(pathToResultsDirectory);

    if (!dirRes.exists())
    {
        QString errorMessage = QString("The Directory: ") + pathToResultsDirectory
                + QString(" Does not exist.");

        this->errorMessage(errorMessage);

        return;
    }

    auto censusFileName = "PopulationDemographicsCensus"+censusVintageCombo->currentText()+".shp";

    censusPathLineEdit->setText(pathToResultsDirectory+censusFileName);

    auto res = this->importCensusDemographicsLayer();
    if(res != 0)
        this->errorMessage("Error importing the population demographics layer");

    auto ACSFileName = "HouseholdIncomeACS"+ACSVintageCombo->currentText()+".shp";

    ACSPathLineEdit->setText(pathToResultsDirectory+ACSFileName);

    res = this->importACSIncomeLayer();
    if(res != 0)
        this->errorMessage("Error importing the ACS household income layer");
}


void HousingUnitAllocationWidget::handleProcessStarted(void)
{
    this->statusMessage("Running script in the background to download census and ACS data. This may take a while.");
    QApplication::processEvents();

    this->runGetCensusButton->setEnabled(false);
}


void HousingUnitAllocationWidget::handleProcessTextOutput(void)
{
    QByteArray output = process->readAllStandardOutput();

    this->statusMessage(QString(output));
}


void HousingUnitAllocationWidget::handleCensusLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(censusBlockLayer)
        censusBlockLayer->setCrs(val);
}


void HousingUnitAllocationWidget::handleACSLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(ACSBlockGroupLayer)
        ACSBlockGroupLayer->setCrs(val);
}


void HousingUnitAllocationWidget::handleBuildingLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(assetLayer)
        assetLayer->setCrs(val);
}


void HousingUnitAllocationWidget::getStandardOutputDir(void)
{
    QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

    if(workingDir.isEmpty())
    {
        QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

        this->errorMessage(errorMessage);

        return;
    }

    QDir dirWork(workingDir);

    if (!dirWork.exists())
        if (!dirWork.mkpath(workingDir))
        {
            QString errorMessage = QString("Could not load the Input File Directory: ") + workingDir
                    + QString(". Change the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return;
        }


    QString outputFilePath = workingDir + QDir::separator() + "HousingDemographics";

    QDir dirGISPoutput(outputFilePath);

    if (!dirGISPoutput.exists())
        if (!dirGISPoutput.mkpath(outputFilePath))
        {
            QString errorMessage = QString("Could not create Directory: ") + outputFilePath;
            this->errorMessage(errorMessage);

            return;
        }

    gisDirLineEdit->setText(outputFilePath);
}


void HousingUnitAllocationWidget::browseGISresultsFolderDir(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    QString tmp = dialog.getExistingDirectory(this,tr("Folder with GIS files"),QString(QDir(gisDirLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
    {
        gisDirLineEdit->setText(tmp);
    }
}


void HousingUnitAllocationWidget::browseBuildingGISFile(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString tmp = dialog.getOpenFileName(this,tr("GIS file of asset"),QString(QDir(buildingsPathLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
        buildingsPathLineEdit->setText(tmp);
    else
        return;

    auto res = this->importBuidlingsLayer();
    if(res != 0)
        this->errorMessage("Error importing the asset layer");
}


void HousingUnitAllocationWidget::browseCensusGISFile(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString tmp = dialog.getOpenFileName(this,tr("GIS file containing census data"),QString(QDir(censusPathLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
        censusPathLineEdit->setText(tmp);
    else
        return;

    auto res = this->importCensusDemographicsLayer();
    if(res != 0)
        this->errorMessage("Error importing the census demographics layer");
}


void HousingUnitAllocationWidget::browseACSGISFile(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString tmp = dialog.getOpenFileName(this,tr("GIS file containing ACS data"),QString(QDir(ACSPathLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
        ACSPathLineEdit->setText(tmp);
    else
        return;

    auto res = this->importACSIncomeLayer();
    if(res != 0)
        this->errorMessage("Error importing the ACS household income layer");

}


void HousingUnitAllocationWidget::handleCreateLayerMainThread(const QString& path, const QString& name)
{
    auto layer = theVisualizationWidget->addVectorLayer(path, name, "ogr");

    Layermap.insert(name,layer);

    emit emitLayerCreationFinished();
}



void HousingUnitAllocationWidget::handleGetLayersFromMap(void)
{
    auto canvas = theVisualizationWidget->getMainCanvas();

    QList<QgsMapLayer *> layers = canvas->layers();

    layerNameCombo->clear();

    // Need this to prevent layer from automatically selecting when combobox is updated
    layerNameCombo->blockSignals(true);

    for(auto&& it : layers)
    {
        auto layerName = it->name();
        layerNameCombo->addItem(layerName);
    }

    layerNameCombo->blockSignals(false);
}


void HousingUnitAllocationWidget::handleSelectLayerFromMap(const QString& selectedLayerName)
{

    // Quick return if
    if(selectedLayerName.compare(selectedLayerNameLabel->text()) == 0)
        return;

    auto canvas = theVisualizationWidget->getMainCanvas();

    QList<QgsMapLayer *> layers = canvas->layers();

    for(auto&& it : layers)
    {
        auto layerName = it->name();

        if(selectedLayerName.compare(layerName) == 0)
        {

            // Need to check that it is a vector layer, i.e., that it has features we can add attributes to
            auto origLayer = dynamic_cast<QgsVectorLayer*>(it);

            if(origLayer == nullptr)
            {
                this->errorMessage("The layer "+layerName+ " must be a vector layer with features to be used in HousingUnitAllocationWidget");
                selectedLayerNameLabel->setText("None Selected");

                return;
            }

            this->infoMessage("Selected layer "+layerName+" in housing unit allocation widget. Creating a copy to modify.");

            // Create a duplicate layer since we will be modifying it
            assetLayer = theVisualizationWidget->duplicateExistingLayer(origLayer);

            if(assetLayer == nullptr)
            {
                this->errorMessage("Error copying the asset layer");
                return;
            }

            selectedLayerNameLabel->setText(layerName);

            break;
        }
    }
}


void HousingUnitAllocationWidget::handleProcessStartedMainThread(const QString& pythonPath, const QStringList& args)
{
    this->getProgressDialog()->showProgressBar();

    process->start(pythonPath, args);
    //process->waitForStarted();
    process->waitForFinished();
}


bool HousingUnitAllocationWidget::downloadCountyFiles(const QString& path)
{

    if(downloadingCensus == true)
        return false;

    QString dlUrl = "https://www2.census.gov/geo/tiger/TIGER2021/COUNTY/tl_2021_us_county.zip";

    emit emitStatusMsg("Downloading county file from "+dlUrl+" please wait");

    emit emitDownloadMainThread(dlUrl,path);

    return true;
}


void HousingUnitAllocationWidget::handleDownloadMainThread(const QString& url, const QString& path)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = fileDownloadManager.get(request);

    reply->setProperty("FilePath",path);

    connect(reply,&QNetworkReply::downloadProgress, this, &HousingUnitAllocationWidget::handleDownloadProgress);
    connect(reply,&QNetworkReply::errorOccurred, this, &HousingUnitAllocationWidget::handleDownloadError);

    downloadingCensus = true;
}


void HousingUnitAllocationWidget::handleDownloadFinished(QNetworkReply* reply)
{

    downloadingCensus = false;

    if(reply)
    {
        emit emitStatusMsg("Download complete");
    }
    else
    {
        emit emitErrorMsg("Download failed");
        return;
    }

    auto filePath = reply->property("FilePath").toString();

    if(filePath.isEmpty())
    {
        emit emitErrorMsg("Empty file path could not save file");
        reply->deleteLater();
        return;
    }


    // Try to open the file path for writing
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        QString err = "Could not open %s for writing: %s\n" + filePath + file.errorString();
        emit emitErrorMsg(err);
        reply->deleteLater();
        return;
    }

    file.write(reply->readAll());
    file.close();

    emit emitStatusMsg("Unzipping file "+filePath);

    // Now unzip the file
    auto pathToOutputDirectory = QCoreApplication::applicationDirPath() +
            QDir::separator() + "Databases" + QDir::separator() + "USCounties2021" + QDir::separator();

    bool result =  ZipUtils::UnzipFile(filePath, pathToOutputDirectory);
    if (result == false)
    {
        QString err = "Error in unziping the downloaded example files";
        emit emitErrorMsg(err);
        reply->deleteLater();
        return;
    }

    reply->deleteLater();

    // Remove the zip file
    QFile zipFile (filePath);
    zipFile.remove();

    emit emitStatusMsg("File unzipping finished. Click on 'Download Census Data' again to run census data download");
}


void HousingUnitAllocationWidget::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(bytesReceived==0 && bytesTotal==0)
    {
        emit emitErrorMsg("Download error");

    }

    emit emitStatusMsg("Download progress "+QString::number(static_cast<int>(static_cast<double>(bytesReceived)/bytesTotal * 100.0)) + "%");
}


void HousingUnitAllocationWidget::handleDownloadError(QNetworkReply::NetworkError code)
{
    emit emitErrorMsg("Error in download with code "+QString(code));
}
