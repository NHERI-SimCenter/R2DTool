#ifndef SITESCATTERWIDGET_H
#define SITESCATTERWIDGET_H
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

// Written by: Kuanshi Zhong, Stevan Gavrilovic

#include "SiteScatter.h"
#include "SimCenterWidget.h"

class QPushButton;
class QLineEdit;
class QTableWidget;

class SiteScatterWidget : public SimCenterWidget
{
    Q_OBJECT
public:
    explicit SiteScatterWidget(SiteScatter& siteScatter, QWidget *parent = nullptr);

    bool getFileLoaded() const;

    bool copySiteFile(const QString& destinationDir);

    bool siteFileExists();

    int getMinID();
    int getMaxID();

signals:

    void viewSiteOnMap();

public slots:

    void loadSiteFile();

    //void viewSiteButtonPressed();

private:

    SiteScatter& m_siteScatter;

    int minID;
    int maxID;

    void setMinID();
    void setMaxID();

    void setupConnections();

    QPushButton* uploadSiteButton;
    QLineEdit* minIDLineEdit;
    QLineEdit* maxIDLineEdit;

    QLineEdit* FilenameLineEdit;
    QString siteFilePath;
    QPushButton* browseFileButton;
    void setSiteFile(QString dirPath);

    // Functions to parse site information from the csv file
    int parseSiteFile(const QString& pathToFile);

    // Functions to parse csv files
    QVector<QStringList> parseCSVFile(const QString &string);
    QStringList parseLineCSV(const QString &string);
    void updateSiteSpreadSheet(const QList<UserSpecifiedSite>& siteList);
    bool updatingSiteTable;
    QStringList defaultCSVHeader;

    // Site file attributes
    QMap<QString, int> attributeIndex;

    QTableWidget *siteSpreadSheet;

    bool fileLoaded;

    //QPushButton* viewSiteButton;

};

#endif // SITESCATTERWIDGET_H
