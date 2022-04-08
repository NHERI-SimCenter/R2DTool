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
#include <Utils/PythonProgressDialog.h>
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

#include <qgsprojectionselectionwidget.h>
#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsgeometryengine.h>
#include <qgsproject.h>

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
    //    auto buildingsGISFile =  "/Users/steve/Desktop/SanFranciscoTestbed/SanFranciscoBuildingFootprints/SanFrancisco_buildingfootprints_2014.shp";

    //    buildingsPathLineEdit->setText(buildingsGISFile);
    //    this->importBuidlingsLayer();
    //    buildingCrsSelector->setCrs(QgsCoordinateReferenceSystem("ESRI:102643"));

    //    this->createGISFiles();
    // Test to remove end
}


HousingUnitAllocationWidget::~HousingUnitAllocationWidget()
{

}


std::set<QString> HousingUnitAllocationWidget::getCountiesFromBuildingInventory(void)
{

    std::set<QString> res;

    if(buildingsLayer == nullptr)
    {
        emit emitErrorMsg("The buildings layer does not exist, please import the buildings");
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

        auto success = downloadCountyFiles(savePath);

        return res;
    }

    emit emitStatusMsg("Getting counties for the building inventory.");

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

    // Iterate through the building features
    auto countyFeatures = countiesLayer->getFeatures();

    QgsFeature countyFeat;

    // Coordinate transformation to transform from the county layer crs to the building layer crs - in the case where they are different
    if(buildingsLayer->crs() != countiesLayer->crs())
    {
        QgsCoordinateTransform coordTrans(countiesLayer->crs(), buildingsLayer->crs(), QgsProject::instance());
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
    auto features = buildingsLayer->getFeatures();

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

    emit emitStatusMsg("Done getting counties for the building inventory.");

    return res;
}


void HousingUnitAllocationWidget::handleCreateGISFilesButtonPressed(void)
{
    thread.release();
    thread = std::make_unique<std::thread>(&HousingUnitAllocationWidget::createGISFiles, this);
}


int HousingUnitAllocationWidget::createGISFiles(void)
{
    if(buildingsLayer == nullptr)
    {
        emit emitErrorMsg("The buildings layer does not exist, please import the buildings");

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

    auto vintage = censusVintageCombo->currentText();

    QJsonObject configFile;

    configFile.insert("OutputDirectory",outputDir);
    configFile.insert("CountiesArray",countiesArr);
    configFile.insert("Vintage",vintage);

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

    //    qDebug()<<"Census Command:"<<args[0]<<" "<<args[1]<<" "<<args[2];

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
    this->statusMessage("Importing building layer.");

    auto path = buildingsPathLineEdit->text();

    auto origBuildingLayer = theVisualizationWidget->addVectorLayer(path, "Buildings", "ogr");

    if(origBuildingLayer == nullptr)
    {
        this->errorMessage("Error importing the building layer");
        return -1;
    }

    origBuildingLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    // Create a duplicate layer since we will be modifying it
    buildingsLayer = theVisualizationWidget->duplicateExistingLayer(origBuildingLayer);

    if(buildingsLayer == nullptr)
    {
        this->errorMessage("Error copying the building layer");
        return -1;
    }

    this->statusMessage("Importing building layer complete.");

    return 0;
}


int HousingUnitAllocationWidget::importCensusBlockLayer(void)
{

    this->statusMessage("Importing census block layer.");

    auto path = blockLevelPathLineEdit->text();

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        this->errorMessage("Error the file does not exist at the path: "+path);
        return -1;
    }


    // Create the census block layer
    blockLayer = theVisualizationWidget->addVectorLayer(path, "Census Blocks", "ogr");

    if(blockLayer == nullptr)
    {
        this->errorMessage("Error creating the census block layer");
        return -1;
    }

    mblockLevelCrsSelector->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));
    blockLayer->setOpacity(0.50);

    this->statusMessage("Importing census block layer complete.");

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

    auto future2 = std::async(&HousingUnitAllocationWidget::getBuildingFeatures, this);

    if(future2.get() != 0)
    {
        emit emitErrorMsg("Error getting the building features");
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
    emit emitStatusMsg("Parsing buildings.");

    buildingLayerSemaphore.lock();

    if(buildingsLayer == nullptr)
    {
        emit emitErrorMsg("Error: no buildings layer");
        buildingLayerSemaphore.unlock();

        return -1;
    }

    auto features = buildingsLayer->getFeatures();

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

    emit emitStatusMsg("Loaded "+QString::number(buildingsLayer->featureCount())+" buildings");

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
    QgsCoordinateTransform ct(parcelsLayer->crs(), buildingsLayer->crs(), QgsProject::instance());
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
    emit emitStatusMsg("Extracting census data.");

    // Lock access to the building layer
    std::unique_lock<std::mutex> lck (buildingLayerSemaphore);

    if(blockLayer == nullptr || buildingsLayer == nullptr)
    {
        emit emitErrorMsg("Error in extracting census data. Either a block layer or buildings layer is missing.");
        return -1;
    }

    auto blockLayerFields = blockLayer->fields();

    QStringList fieldNames;
    for(int i = 0; i<blockLayerFields.count(); ++i)
    {
        auto field = blockLayerFields.at(i);
        auto fieldName = field.name();

        if(fieldName.compare("fid") == 0)
            continue;

        fieldNames.append("BLOCKLAYER_"+fieldName);
    }

    std::vector<QgsFeature> blockFeatVec;
    blockFeatVec.reserve(blockLayer->featureCount());

    auto blockFeatures = blockLayer->getFeatures();
    QgsFeature blockFeat;

    // Coordinate transformation to transform from the block layer crs to the building layer crs - in the case where they are different
    if(buildingsLayer->crs() != blockLayer->crs())
    {
        QgsCoordinateTransform coordTrans(blockLayer->crs(), buildingsLayer->crs(), QgsProject::instance());
        while (blockFeatures.nextFeature(blockFeat))
        {
            auto geom = blockFeat.geometry();
            geom.get()->transform(coordTrans);
            blockFeat.setGeometry(geom);
            blockFeatVec.push_back(blockFeat);
        }
    }
    else
    {
        while (blockFeatures.nextFeature(blockFeat))
            blockFeatVec.push_back(blockFeat);
    }

    auto numFeat = buildingsLayer->featureCount();

    if(numFeat == 0)
    {
        emit emitErrorMsg("Error, number of features is zero in the buildings layer "+buildingsLayer->name());
        return -1;
    }

    QVector< QgsAttributes > fieldAttributes(numFeat, QgsAttributes(fieldNames.size()));

    // Need to return the features with ascending ids so that when we set the updated features to the layer, things will be in order
    QgsFeatureRequest featRequest;
    QgsFeatureRequest::OrderByClause orderByClause(QString("id"),true);
    QList<QgsFeatureRequest::OrderByClause> obcList = {orderByClause};
    QgsFeatureRequest::OrderBy orderBy(obcList);
    featRequest.setOrderBy(orderBy);

    auto buildingFeatures = buildingsLayer->getFeatures(featRequest);
    QgsFeature buildingFeat;
    int count = 0;
    while (buildingFeatures.nextFeature(buildingFeat))
    {
        // Get the building centroid
        auto buildCentroidPoint = buildingFeat.geometry().centroid().asPoint();

        // Did we find the information that we are looking for
        bool found = false;

        //Iterate through the blockLayer features
        for(auto&& it : blockFeatVec)
        {
            auto blockGeom = it.geometry();

            // Do bounding box check which is very fast to check if building is within a block group
            auto bb = blockGeom.boundingBox();
            if(bb.contains(buildCentroidPoint))
            {
                QgsAttributes featAtrbs = it.attributes();

                //                auto fieldIDx = it.fieldNameIndex("fid");

                for(int i = 0; i<featAtrbs.size(); ++i)
                {
                    auto atrb = featAtrbs.at(i);

                    fieldAttributes[count][i] = atrb;
                }

                found = true;
                break;
            }
        }

        if(!found)
        {
            emit emitErrorMsg("Error: could not find a census block for the feature  "+QString::number(buildingFeat.id()));
            return -1;
        }

        ++count;
    }

    if(fieldNames.size() != fieldAttributes.front().size())
    {
        emit emitErrorMsg("Error: inconsistency between the field names and number of attributes  "+QString::number(buildingFeat.id()));
        return -1;
    }

    QString errMsg;
    auto res = theVisualizationWidget->addNewFeatureAttributesToLayer(buildingsLayer,fieldNames,fieldAttributes,errMsg);
    if(res != 0)
    {
        emit emitErrorMsg(errMsg);
        return -1;
    }

    emit emitStatusMsg("Done extracting census data.");

    return 0;
}


void HousingUnitAllocationWidget::clear()
{
    Layermap.clear();
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

    QLabel* buildingPathText = new QLabel("Path to building GIS file (.gdb, .shp, etc.)");
    buildingsPathLineEdit = new QLineEdit();
    QPushButton *browseBuidlingFileButton = new QPushButton("Browse");

    connect(browseBuidlingFileButton,SIGNAL(clicked()),this,SLOT(browseBuildingGISFile()));

    QLabel* buildCrsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    buildingCrsSelector = new QgsProjectionSelectionWidget();
    buildingCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    buildingCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    buildingCrsSelector->setFocusPolicy(Qt::StrongFocus);

    connect(buildingCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,&HousingUnitAllocationWidget::handleBuildingLayerCrsChanged);

    auto buildingHBox = new QHBoxLayout();
    buildingHBox->addWidget(buildingPathText);
    buildingHBox->addWidget(buildingsPathLineEdit);
    buildingHBox->addWidget(browseBuidlingFileButton);
    buildingHBox->addWidget(buildCrsTypeLabel);
    buildingHBox->addWidget(buildingCrsSelector);


    mblockLevelCrsSelector = new QgsProjectionSelectionWidget();
    mblockLevelCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    mblockLevelCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    mblockLevelCrsSelector->setFocusPolicy(Qt::StrongFocus);

    connect(mblockLevelCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,&HousingUnitAllocationWidget::handleBlockLayerCrsChanged);

    QGroupBox* censusDataGB = new QGroupBox("Download Census Data (create .gdb files from scratch)");
    QGridLayout* cdGBLayout = new QGridLayout(censusDataGB);

    QLabel* censusVintageLabel = new QLabel("Census date:");
    censusVintageCombo = new QComboBox();
    censusVintageCombo->addItems(QStringList({"2000","2010"}));
    censusVintageCombo->setCurrentText("2010");

    // Output dir where the results will be stored
    QLabel* gisDirLabel = new QLabel(tr("Directory to store created .GIS files"),this);
    gisDirLineEdit = new QLineEdit();

    // Populate the standard output dir
    this->getStandardOutputDir();

    auto getGISFolderButton = new QPushButton(tr("Browse"),this);
    connect(getGISFolderButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::browseGISresultsFolderDir);

    runGetCensusButton = new QPushButton("Download Census Data");
    connect(runGetCensusButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::handleCreateGISFilesButtonPressed);

    cdGBLayout->addWidget(censusVintageLabel,0,0);
    cdGBLayout->addWidget(censusVintageCombo,0,1,1,2);

    cdGBLayout->addWidget(gisDirLabel,1,0);
    cdGBLayout->addWidget(gisDirLineEdit,1,1);
    cdGBLayout->addWidget(getGISFolderButton,1,2);

    cdGBLayout->addWidget(runGetCensusButton,2,0,1,3);

    QLabel* selectPathText = new QLabel("Census Block-level GIS file (.gdb, .shp, etc.) for population demographics");
    blockLevelPathLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(browseBlockLevelGISFile()));

    mainLayout->addLayout(buildingHBox,0,0,1,5);
    mainLayout->addWidget(censusDataGB, 1,0,1,5);

    mainLayout->addWidget(selectPathText, 2,0);
    mainLayout->addWidget(blockLevelPathLineEdit, 2,1);
    mainLayout->addWidget(browseFileButton, 2,2);

    QLabel* crsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    mainLayout->addWidget(crsTypeLabel,2,3);
    mainLayout->addWidget(mblockLevelCrsSelector,2,4);

    runJoinButton = new QPushButton("Run");
    connect(runJoinButton, &QPushButton::clicked, this, &HousingUnitAllocationWidget::handleRunJoinButtonPressed);

    mainLayout->addWidget(runJoinButton,3,0);
    mainLayout->setRowStretch(3,1);

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

    auto fileName = "CensusData"+censusVintageCombo->currentText()+".shp";

    blockLevelPathLineEdit->setText(pathToResultsDirectory+fileName);

    this->importCensusBlockLayer();
}


void HousingUnitAllocationWidget::handleProcessStarted(void)
{
    this->statusMessage("Running script in the background");
    this->runGetCensusButton->setEnabled(false);
}


void HousingUnitAllocationWidget::handleProcessTextOutput(void)
{
    QByteArray output = process->readAllStandardOutput();

    this->statusMessage(QString(output));
}


void HousingUnitAllocationWidget::handleBlockLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(blockLayer)
        blockLayer->setCrs(val);
}


void HousingUnitAllocationWidget::handleBuildingLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(buildingsLayer)
        buildingsLayer->setCrs(val);
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


    QString outputFilePath = workingDir + QDir::separator() + "HousingDemograhics";

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
    QString tmp = dialog.getOpenFileName(this,tr("GIS file of building"),QString(QDir(buildingsPathLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
        buildingsPathLineEdit->setText(tmp);
    else
        return;

    auto res = this->importBuidlingsLayer();
    if(res != 0)
        this->errorMessage("Error importing the building layer");
}


void HousingUnitAllocationWidget::browseBlockLevelGISFile(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString tmp = dialog.getOpenFileName(this,tr("GIS file of block level data"),QString(QDir(buildingsPathLineEdit->text()).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
        blockLevelPathLineEdit->setText(tmp);
    else
        return;

    auto res = this->importCensusBlockLayer();
    if(res != 0)
        this->errorMessage("Error importing the census block layer");

}


void HousingUnitAllocationWidget::handleCreateLayerMainThread(const QString& path, const QString& name)
{
    auto layer = theVisualizationWidget->addVectorLayer(path, name, "ogr");

    Layermap.insert(name,layer);

    emit emitLayerCreationFinished();
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
