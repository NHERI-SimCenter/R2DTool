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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Stevan Gavrilovic

#include "NGAW2Converter.h"
#include "CSVReaderWriter.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

#include <math.h>

NGAW2Converter::NGAW2Converter()
{
    directionH1 = true;
    directionH2 = true;
    directionVert = false;
}


int NGAW2Converter::convertToSimCenterEvent(const QString& pathToOutputDirectory, const QJsonObject& NGA2Results, QString& errorMsg, QJsonObject* createdRecords)
{

    const QFileInfo inputFilesInfo(pathToOutputDirectory);

    if (!inputFilesInfo.exists() || inputFilesInfo.isFile())
    {
        errorMsg ="A directory does not exist at the path: " + pathToOutputDirectory;
        return -1;
    }

    QStringList acceptableFileExtensions = {"*.AT2", "*.VT2", "*.DT2"};

    QStringList inputFiles = inputFilesInfo.dir().entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        errorMsg ="No files with .AT2, *.VT2, or *.DT2 extensions were found at the path: " + pathToOutputDirectory;
        return -2;
    }


    if(!directionH1 && !directionH2 && !directionVert)
    {
        errorMsg = "No directions specified. Please set H1, H2, or Vert to ON";
        return -1;
    }


    // Get the file names for the various component-directions
    auto metaData = NGA2Results.value("-- Summary of Metadata of Selected Records --").toObject();

    auto records = metaData.keys();

    for(auto&& it : records)
    {
        auto recordObj = metaData[it].toObject();

        auto RSNNumber = recordObj.value("Record Sequence Number").toString();

        if(RSNNumber.isEmpty())
        {
            errorMsg = "Empty record sequence number";
            return -1;
        }

        auto name = "RSN"+RSNNumber;

        auto H1FileName = recordObj.value("Horizontal-1 Acc. Filename").toString();
        auto H2FileName = recordObj.value("Horizontal-2 Acc. Filename").toString();
        auto VFileName = recordObj.value("Vertical Acc. Filename").toString();

        if(H1FileName.isEmpty() || H2FileName.isEmpty() || VFileName.isEmpty())
        {
            errorMsg = "Empty time history file name";
            return -1;
        }

        QJsonObject recordJsonObj;

        recordJsonObj.insert("name",name);

        auto dT = -1.0;

        if(directionH1)
        {
            auto filePathH1 = pathToOutputDirectory + H1FileName;
            QJsonObject H1Obj;
            auto res1 = this->convertRecordToJson(filePathH1,H1Obj,errorMsg);
            if(res1 != 0)
            {
                errorMsg = "Error importing file " + filePathH1;
                return -1;
            }

            // Get the time step
            auto dTTs = H1Obj.value("dT").toDouble();
            dT = dTTs;

            // Get the time history data points
            auto TH = H1Obj.value("TimeHistoryData").toArray();
            recordJsonObj.insert("data_x",TH);

            auto PGA = this->getPGA(&TH);
            recordJsonObj.insert("PGA_x",PGA);
        }

        if(directionH2)
        {
            auto filePathH2 = pathToOutputDirectory + H2FileName;
            QJsonObject H2Obj;
            auto res2 = this->convertRecordToJson(filePathH2,H2Obj,errorMsg);
            if(res2 != 0)
            {
                errorMsg = "Error importing file " + filePathH2;
                return -1;
            }

            // Set the time step if not already set
            auto dTTs = H2Obj.value("dT").toDouble();
            if(dT < 0.0)
                dT = dTTs;
            else
            {
                // Check if the time step is the same for all time history files
                if(fabs(dTTs-dT) > 1.0e-6)
                {
                    errorMsg = "Error, inconsistent time step size in the time history files.";
                    return -1;
                }
            }

            // Get the time history data points
            auto TH = H2Obj.value("TimeHistoryData").toArray();
            recordJsonObj.insert("data_y",TH);

            auto PGA = this->getPGA(&TH);
            recordJsonObj.insert("PGA_y",PGA);
        }

        if(directionVert)
        {
            auto filePathV = pathToOutputDirectory + VFileName;
            QJsonObject VObj;
            auto res3 = this->convertRecordToJson(filePathV,VObj,errorMsg);
            if(res3 != 0)
            {
                errorMsg = "Error importing file " + filePathV;
                return -1;
            }

            auto dTTs = VObj.value("dT").toDouble();

            if(dT < 0.0)
                dT = dTTs;
            else
            {
                // Check if the time step is the same for all time history files
                if(fabs(dTTs-dT) > 1.0e-6)
                {
                    errorMsg = "Error, inconsistent time step size in the time history files.";
                    return -1;
                }
            }

            // Get the time history data points
            auto TH = VObj.value("TimeHistoryData").toArray();
            recordJsonObj.insert("data_z",TH);

            auto PGA = this->getPGA(&TH);
            recordJsonObj.insert("PGA_z",PGA);
        }

        if(dT <= 0.0)
        {
            errorMsg = "Error getting the time step from the time history files";
            return -1;
        }

        recordJsonObj.insert("dT",dT);

        QString outputFile = pathToOutputDirectory + name + ".json";

        QFile file(outputFile);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            errorMsg = "Error creating the output json file";
            return -1;
        }

        // Write the file to the folder
        QJsonDocument doc(recordJsonObj);
        file.write(doc.toJson());
        file.close();

        if(createdRecords)
        {
            createdRecords->insert("name",recordJsonObj);
        }
    }

    // Remove the raw files
    for(auto&& it : inputFiles)
    {
        QFile file(pathToOutputDirectory + it);
        file.remove();
    }

    return 0;
}


int NGAW2Converter::parseNGAW2SearchResults(const QString& filesDirectoryPath, QJsonObject& resultsJson, QString& errorMsg)
{

    auto pathToSearchResultsFile = filesDirectoryPath + QDir::separator() + "_SearchResults.csv";

    const QFileInfo resultsFileInfo(pathToSearchResultsFile);

    if (!resultsFileInfo.exists() || !resultsFileInfo.isFile())
    {
        errorMsg ="The _SearchResults.csv file does not exist at the path: " + filesDirectoryPath;
        return -1;
    }

    // Import the data from the _SearchResults.csv file
    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToSearchResultsFile,err);

    if(!err.isEmpty())
    {
        errorMsg = err;
        return -1;
    }

    if(data.empty())
    {
        errorMsg ="The _SearchResults.csv file is empty";
        return -1;
    }

    // Summary information
    auto summaryVec = data.mid(5,24);

    QJsonObject summaryObj;

    for(auto&& it : summaryVec)
    {
        auto key = it.at(0);

        auto value = it.at(1);

        summaryObj.insert(key,value);
    }

    resultsJson.insert(data.at(4).first(),summaryObj);

    // Meta data of records
    QJsonObject recordsObj = resultsJson.value(data.at(32).first()).toObject();

    auto columnHeadings = data.at(33);

    for(int i = 34; i<data.size(); ++i)
    {
        auto rowData = data.at(i);

        // Stop if the first cell is not an integer id
        if(rowData.first().toInt() == 0)
            break;

        if(rowData.size() != columnHeadings.size())
        {
            errorMsg ="Inconsistencies in the NGAW2 data";
            return -1;
        }

        QJsonObject resultObj;

        for(int j = 1; j<rowData.size(); ++j)
        {
            auto key = columnHeadings.at(j);
            auto value = rowData.at(j);

            resultObj.insert(key,value);
        }

        recordsObj.insert(rowData.at(2),std::move(resultObj));
    }

    resultsJson.insert(data.at(32).first(),recordsObj);

    return 0;
}


int NGAW2Converter::convertRecordToJson(const QString& inputFile, QJsonObject& recordJson, QString& errorMsg)
{
    // Open the raw file
    QFile theRecordFile = QFile(inputFile);

    if (!theRecordFile.exists())
    {
        errorMsg = QString("No file ") +  inputFile + QString(" exists");
        return -1;
    }

    if (theRecordFile.open(QIODevice::ReadOnly))
    {
        auto firstLine = theRecordFile.readLine();

        if(firstLine.compare("PEER NGA STRONG MOTION DATABASE RECORD\r\n") != 0)
        {
            errorMsg = "Only PEER NGA files supported";
            return -1;
        }

        // Get the second line -> event name, event date, station ID, direction
        auto secondLine = theRecordFile.readLine();

        auto secondLineValues = secondLine.split(',');

        if(secondLineValues.size() != 4)
        {
            errorMsg = "Error importing the time series raw data";
            return -1;
        }

        auto eventName = QString::fromLocal8Bit(secondLineValues.at(0)).trimmed();

        auto eventDate = QString::fromLocal8Bit(secondLineValues.at(1)).trimmed();

        auto stationID = QString::fromLocal8Bit(secondLineValues.at(2)).trimmed();

        auto direction = QString::fromLocal8Bit(secondLineValues.at(3)).trimmed();

        // Get the third line - type of time history, acceleration, velocity, displacement, etc.
        auto timeHistoryType = QString::fromLocal8Bit(theRecordFile.readLine()).trimmed();

        // Get the fourth line - number of points and time step (Dt)
        auto fourthLine = QString::fromLocal8Bit(theRecordFile.readLine()).trimmed();

        QRegExp rx = QRegExp("NPTS=\\s*([1-9][0-9]*)\\s*,\\s*DT=\\s*(\\d*\\.\\d+)\\s*SEC");

        rx.indexIn(fourthLine);

        QStringList qsl = rx.capturedTexts();

        if(qsl.size() != 3)
            return -1;

        bool OK = true;

        auto numPtnsStr = qsl[1];
        auto numPnts = numPtnsStr.toInt(&OK);

        if(!OK)
        {
            errorMsg = "Error converting string to integer";
            return -1;
        }

        auto dTStr = qsl[2];
        auto dT = dTStr.toDouble(&OK);

        if(!OK)
        {
            errorMsg = "Error converting string to double";
            return -1;
        }

        QJsonArray timeHistoryArray;

        QRegExp delim("\\s+");

        // File opened successfully, parse csv file for records
        while (!theRecordFile.atEnd())
        {
            auto line =  QString::fromLocal8Bit(theRecordFile.readLine()).trimmed();

            foreach (const QString &item, line.split(delim, QString::SkipEmptyParts))
            {
                bool OK = true;
                auto dataPointValue = item.toDouble(&OK);

                if(!OK)
                {
                    errorMsg = "Error converting to double ";
                    return -1;
                }

                timeHistoryArray.append(dataPointValue);

            }
        }

        if(timeHistoryArray.size() != numPnts)
        {
            errorMsg = "Error, the number of imported points should match the number of points in the time-history input file";
            return -1;
        }

        recordJson.insert("NumberPoints", numPnts);
        recordJson.insert("EventName", eventName);
        recordJson.insert("EventDate", eventDate);
        recordJson.insert("StationID", stationID);
        recordJson.insert("Direction", direction);
        recordJson.insert("TimeHistoryType", timeHistoryType);
        recordJson.insert("dT", dT);
        recordJson.insert("TimeHistoryData",timeHistoryArray);
    }


    return 0;
}


double NGAW2Converter::getPGA(const QJsonArray* timeHistory)
{

    auto variantList = timeHistory->toVariantList();

    auto PGAmax = 0.0;

    for(auto&& it : variantList)
    {
        auto val = it.toDouble();

        if(fabs(val) > PGAmax)
           PGAmax = val;
    }

    return PGAmax;
}

