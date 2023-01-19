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

// Written by: Kuanshi Zhong

#include "OpenQuakeClassical.h"
#include "SimCenterPreferences.h"

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QMessageBox>
#include <QDirIterator>
using namespace ZipUtils;

OpenQuakeClassical::OpenQuakeClassical(int uMode, double rMesh, double aMesh, double maxDist, QString rFile, QWidget *parent) :
    SimCenterWidget(parent), userMode(uMode), rupMesh(rMesh), areaMesh(aMesh), maxDistance(maxDist), sourceFilename(rFile)
{

}


double OpenQuakeClassical::getRupMesh() const
{
    return rupMesh;
}


double OpenQuakeClassical::getAreaMesh() const
{
    return areaMesh;
}


void OpenQuakeClassical::setRupMesh(double value)
{
    if(this->rupMesh != value)
    {
        rupMesh = value;
    }
}


void OpenQuakeClassical::setAreaMesh(double value)
{
    if(this->areaMesh != value)
    {
        areaMesh = value;
    }
}


void OpenQuakeClassical::setMaxDistance(double value)
{
    maxDistance = value;
}


QString OpenQuakeClassical::getRupFilename() const
{
    return sourceFilename;
}


QString OpenQuakeClassical::getGMPEFilename() const
{
    QFileInfo fileInfo(this->gmpeFilename);
    // only reserve the filename
    QString filename(fileInfo.fileName());
    qDebug() << filename;
    return filename;
}


void OpenQuakeClassical::setSourceFilename(const QString &value)
{
    sourceFilename = value;
}

void OpenQuakeClassical::setSourceModelDir(const QString &value)
{
    sourceDefDir = value;
}

void OpenQuakeClassical::setOpenQuakeVersion(const QString &value)
{
    oq_version = value;
}


void OpenQuakeClassical::setGMPEFilename(const QString &value)
{
    gmpeFilename = value;
}

void OpenQuakeClassical::setConfigFilename(const QString &value)
{
    configFilename = value;
}


double OpenQuakeClassical::getMaxDistance() const
{
    return maxDistance;
}


void OpenQuakeClassical::setIndivHC(int state)
{
    indivHC = state > 0;
}


void OpenQuakeClassical::setMeanHC(int state)
{
    meanHC = state > 0;
}


void OpenQuakeClassical::setHazMap(int state)
{
    hazMap = state > 0;
}


void OpenQuakeClassical::setUHS(int state)
{
    UHS = state > 0;
}

void OpenQuakeClassical::setReturnPeriod(const QString &value)
{
    returnPeriod = value.toInt();
}

void OpenQuakeClassical::setTimeSpan(const QString &value)
{
    timeSpan = value.toInt();
}

void OpenQuakeClassical::setRandSeed(const QString &value)
{
    randSeed = value.toInt();
}

void OpenQuakeClassical::setOQVersion(const QString &value)
{
    oq_version = value;
}



void OpenQuakeClassical::setQuantiles(const QString &value)
{
    while(quant.count())
    {
        quant.pop_back();
     }
    // remove spaces
    QString tmp = value;
    tmp.remove(" ");
    // split by ","
    QList<QString> tmpList = tmp.split(",");
    // check size of the list (must be three)
    if (tmpList.count() != 3)
    {
        QString errMsg = QString("Please provide three number in (0,1) for the Quantiles.");
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    // load the data
    for (int i = 0; i != 3; ++i)
    {
        if (tmpList[i].toDouble() <=0 || tmpList[i].toDouble() >=1)
        {
            QString errMsg = QString("Quantiles must be in (0,1).");
            qDebug() << errMsg;
            //this->errorMessage(errMsg);
            return;
        }
        quant.append(tmpList[i].toDouble());
    }
}


bool OpenQuakeClassical::outputToJSON(QJsonObject &jsonObject)
{
    qDebug() << "OpenQuakeClassical: writing rupture JSON...";
    // Extract the filename from the path
    QString filename;
    filename = this->sourceFilename.section('/', -1);

    QJsonObject rupture;
    if (userMode == 0) {
        jsonObject.insert("Type", "OpenQuakeClassicalPSHA");
    } else {
        jsonObject.insert("Type", "OpenQuakeUserConfig");
        jsonObject.insert("ConfigFile", configFilename.section('/', -1));
    }
    jsonObject.insert("Filename",filename);
    jsonObject.insert("RupMesh", rupMesh);
    jsonObject.insert("AreaMesh", areaMesh);
    jsonObject.insert("max_Dist", maxDistance);
    jsonObject.insert("TimeSpan", timeSpan);
    jsonObject.insert("Seed", randSeed);
    jsonObject.insert("IndivHazCurv", indivHC);
    jsonObject.insert("MeanHazCurv", meanHC);
    jsonObject.insert("Quantiles", quant);
    jsonObject.insert("HazMap", hazMap);
    jsonObject.insert("UHS", UHS);
    jsonObject.insert("ReturnPeriod", returnPeriod);
    jsonObject.insert("OQVersion",oq_version);

    //Also need to copy rupture file for OpenQuake runs
    if (! this->copySourceFile())
    {
        QString errMsg = "Cannot copy the source logic tree file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    if (! this->copyGMPEFile())
    {
        QString errMsg = "Cannot copy the GMPE logic tree file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    if (! this->copySourceModelFile())
    {
        QString errMsg = "Cannot copy the Source Model.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    if (userMode == 1) {
        // copy the ini
        if (! this->copyConfigFile())
        {
            QString errMsg = "Cannot copy the configuration file.";
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }
    }

    return true;
}

bool OpenQuakeClassical::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}

bool OpenQuakeClassical::copySourceFile()
{
    // Destination directory
    QString destinationDir;
    destinationDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + QDir::separator() +
            "HazardSimulation" + QDir::separator() + QString("GroundMotions") + QDir::separator() + "Input";
    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }

    // source logic tree file
    QString sfilename = this->sourceFilename;

    QFile fileToCopy(sfilename);

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the source logic tree file: " + QString(sfilename);
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFileInfo fileInfo(sfilename);
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + theFile);
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing source logic tree file in the input directory.";
        qDebug() << warnMsg;
        //this->messageDialog(warnMsg);
    }

    return fileToCopy.copy(destinationDir + QDir::separator() + theFile);
}


bool OpenQuakeClassical::copyGMPEFile()
{
    // Destination directory
    QString destinationDir;
    destinationDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + QDir::separator() +
            "HazardSimulation" + QDir::separator() + QString("GroundMotions") + QDir::separator() + "Input";
    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }

    // source logic tree file
    QString gfilename = this->gmpeFilename;

    QFile fileToCopy(gfilename);

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the GMPE logic tree file." + QString(gfilename);
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFileInfo fileInfo(gfilename);
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + theFile);
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing GMPE logic tree file in the input directory.";
        qDebug() << warnMsg;
        //this->messageDialog(warnMsg);
    }

    return fileToCopy.copy(destinationDir + QDir::separator() + theFile);
}


bool OpenQuakeClassical::copySourceModelFile()
{
    // Destination directory
    QString destinationDir;
    destinationDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + QDir::separator() +
            "HazardSimulation" + QDir::separator() + QString("GroundMotions") + QDir::separator() + "Input";
    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }

    // source logic tree file
    QString sdfilename = this->sourceDefDir;

    qDebug() << "Zipping the source model directory...";
    bool zipTool = ZipFolder(sdfilename, QFileInfo(sdfilename).path() + QDir::separator() + "source_model.zip");

    if (!zipTool)
    {
        QString errMsg = QString("Could not zip the source directory.");
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFile fileToCopy(QFileInfo(sdfilename).path() + QDir::separator() + "source_model.zip");

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the zipped source file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFileInfo fileInfo(QFileInfo(sdfilename).path() + QDir::separator() + "source_model.zip");
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + theFile);
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing source zip file in the input directory.";
        qDebug() << warnMsg;
        //this->messageDialog(warnMsg);
    }

    // copy to the destinationDir
    if (! fileToCopy.copy(destinationDir + QDir::separator() + theFile))
    {
        QString warnMsg = "Cannot copy the source zip file.";
        qDebug() << warnMsg;
        return false;
    }

    // unzip the files in the source zip file
    if (! UnzipFile(QString(destinationDir + QDir::separator() + theFile), destinationDir))
    {
        QString warnMsg = "Cannot unzip the source zip file.";
        qDebug() << warnMsg;
        return false;
    }

    // copy the files to the main input diretctory
    QString sourceFolderName = QString(destinationDir + QDir::separator() + QDir(sdfilename).dirName());

    qDebug() << sourceFolderName;

    QDirIterator curItem(sourceFolderName, QDirIterator::Subdirectories);
    bool copyFlag = true;
    while (curItem.hasNext())
    {
        QFileInfo fileInfo(curItem.next());

        if(fileInfo.isDir())
            continue;

        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        QFile afile(fileInfo.filePath());
        qDebug() << fileInfo.filePath();

        if(afile.exists())
        {
            QString curFilename = fileInfo.fileName();
            qDebug() << QString(destinationDir)+curFilename;
            QFile distFile(destinationDir + QDir::separator() + curFilename);
            // Overwriting check
            if (distFile.exists())
            {
                distFile.remove();
                QString warnMsg = "Overwriting the existing source model file in the input directory.";
                qDebug() << warnMsg;
                //this->messageDialog(warnMsg);
            }
            if (! afile.copy(destinationDir + QDir::separator() + curFilename))
            {
                return false;
            }
        }
    }

    return copyFlag;

}

bool OpenQuakeClassical::copyConfigFile()
{
    // Destination directory
    QString destinationDir;
    destinationDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + QDir::separator() +
            "HazardSimulation" + QDir::separator() + QString("GroundMotions") + QDir::separator() + "Input";
    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }

    // source logic tree file
    QString cfilename = this->configFilename;

    QFile fileToCopy(cfilename);

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the GMPE logic tree file." + QString(cfilename);
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFileInfo fileInfo(cfilename);
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + theFile);
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing configuration file in the input directory.";
        qDebug() << warnMsg;
        //this->messageDialog(warnMsg);
    }

    return fileToCopy.copy(destinationDir + QDir::separator() + theFile);
}

void OpenQuakeClassical::reset(void)
{

}

