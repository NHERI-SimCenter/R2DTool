#ifndef OpenQuakeClassical_H
#define OpenQuakeClassical_H
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

#include "JsonSerializable.h"
#include "SimCenterWidget.h"
#include <QJsonArray>
#include "ZipUtils.h"

class OpenQuakeClassical : public SimCenterWidget, JsonSerializable
{
    Q_OBJECT

public:
    OpenQuakeClassical(int uMode, double rMesh, double aMesh, double maxDist, QString rFile, QWidget *parent = nullptr);

    double getRupMesh() const;
    double getAreaMesh() const;
    double getMaxDistance() const;
    QString getRupFilename() const;
    QString getGMPEFilename() const;
    void setOQVersion(const QString &value);

    //QJsonObject getJson();
    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

signals:

public slots:
    void setRupMesh(double magnitude);
    void setAreaMesh(double magnitude);
    void setMaxDistance(double value);
    void setSourceFilename(const QString &value);
    void setGMPEFilename(const QString &value);
    void setSourceModelDir(const QString &value);
    void setOpenQuakeVersion(const QString &value);
    void setIndivHC(int state);
    void setMeanHC(int state);
    void setHazMap(int state);
    void setUHS(int state);
    void setReturnPeriod(const QString &value);
    void setRandSeed(const QString &value);
    void setQuantiles(const QString &value);
    void setTimeSpan(const QString &value);
    void setConfigFilename(const QString &value);

private:
    int userMode = 0;
    double rupMesh;
    double areaMesh;
    double maxDistance;
    double timeSpan = 1;
    int randSeed = 14;
    bool indivHC = true;
    bool meanHC = true;
    bool hazMap = true;
    bool UHS = true;
    int returnPeriod = 500;
    QJsonArray quant = {0.1,0.5,0.9};
    QString oq_version = "3.12.0"; // default OpenQuake version

    QString sourceFilename;
    QString gmpeFilename;
    QString sourceDefDir; // directory of individual source model xml files
    QString configFilename;

    bool copySourceFile();
    bool copyGMPEFile();
    bool copySourceModelFile();
    bool copyConfigFile();

};

#endif // OpenQuakeClassical_H
