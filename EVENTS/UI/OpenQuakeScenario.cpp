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

#include "OpenQuakeScenario.h"
#include "SimCenterPreferences.h"
#include "GMWidget.h"
#include "GmAppConfig.h"

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QMessageBox>

OpenQuakeScenario::OpenQuakeScenario(double rMesh, double aMesh, double maxDist, QString rFile, QWidget *parent) :
    SimCenterWidget(parent), rupMesh(rMesh), areaMesh(aMesh), maxDistance(maxDist), rupFilename(rFile)
{
    gmWidget = dynamic_cast<GMWidget*>(parent);
}


double OpenQuakeScenario::getRupMesh() const
{
    return rupMesh;
}


double OpenQuakeScenario::getAreaMesh() const
{
    return areaMesh;
}


void OpenQuakeScenario::setRupMesh(double magnitude)
{
    if(this->rupMesh != magnitude)
    {
        rupMesh = magnitude;
    }
}


void OpenQuakeScenario::setAreaMesh(double magnitude)
{
    if(this->areaMesh != magnitude)
    {
        areaMesh = magnitude;
    }
}


void OpenQuakeScenario::setMaxDistance(double value)
{
    maxDistance = value;
}


QString OpenQuakeScenario::getRupFilename() const
{
    return rupFilename;
}


void OpenQuakeScenario::setRupFilename(const QString &value)
{
    rupFilename = value;
}


double OpenQuakeScenario::getMaxDistance() const
{
    return maxDistance;
}


bool OpenQuakeScenario::outputToJSON(QJsonObject &jsonObject)
{
    // Extract the filename from the path
    QString filename;
    filename = this->rupFilename.section('/', -1);

    jsonObject.insert("Type", "OpenQuakeScenario");
    jsonObject.insert("Filename",filename);
    jsonObject.insert("RupMesh", rupMesh);
    jsonObject.insert("AreaMesh", areaMesh);
    jsonObject.insert("max_Dist", maxDistance);

    //Also need to copy rupture file for OpenQuake runs
    if (! this->copyRupFile())
    {
        QString errMsg = "Cannot copy the rupture file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    return true;
}


bool OpenQuakeScenario::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


bool OpenQuakeScenario::copyRupFile()
{

    if(gmWidget == nullptr)
    {
        QString errMsg = QString("Could not get the GMWidget.");
        this->errorMessage(errMsg);
        return false;
    }

    // Destination directory
    QString destinationDir = gmWidget->appConfig()->getInputDirectoryPath() + QDir::separator();

    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }

    QString filename = this->rupFilename;

    QFile fileToCopy(filename);

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the rupture file." + QString(filename);
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFileInfo fileInfo(filename);
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + theFile);
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing rupture file in the input directory.";
        qDebug() << warnMsg;
        //this->messageDialog(warnMsg);
    }

    return fileToCopy.copy(destinationDir + QDir::separator() + theFile);
}


void OpenQuakeScenario::reset(void)
{
    gmWidget->clear();
}

