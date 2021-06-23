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
#include "SiteScatterWidget.h"
#include "HBoxFormLayout.h"
#include "SimCenterPreferences.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QGroupBox>
#include <QTableWidget>
#include <QFileDialog>
#include <QHeaderView>

SiteScatterWidget::SiteScatterWidget(SiteScatter& siteScatter, QWidget *parent) : SimCenterWidget(parent), m_siteScatter(siteScatter)
{
    fileLoaded = false;

    //We use a grid layout for the site widget
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Site File box
    QLabel* filenameLabel = new QLabel(tr("Site File (.csv):"),this);
    FilenameLineEdit = new QLineEdit(this);
    FilenameLineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    browseFileButton = new QPushButton();

    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    // Minimum and Maximum ID in the site csv file: will be passed into
    // hazard simulation app (default numbers are 0, and 100000
    // users could give flexible ranges if they want to run all sites
    // hazard simulation app would do min and max check in the runtime
    QLabel* minIDLabel = new QLabel(tr("Min ID:"),this);
    QLabel* maxIDLabel = new QLabel(tr("Max ID:"),this);

    minIDLineEdit = new QLineEdit("0");
    maxIDLineEdit = new QLineEdit("1000000");
    // Default min and max IDs
    this->setMinID();
    this->setMaxID();
    minIDLineEdit->setValidator(new QIntValidator(0, 1000000, this));
    maxIDLineEdit->setValidator(new QIntValidator(0, 1000000, this));

    minIDLineEdit->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    maxIDLineEdit->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    // Preview of the site file
    auto previewGroup = new QGroupBox(tr("Preview Sites"));
    auto previewLayout = new QGridLayout(previewGroup);
    siteSpreadSheet = new QTableWidget();
    siteSpreadSheet->verticalHeader()->setVisible(false);
    siteSpreadSheet->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    siteSpreadSheet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    previewLayout->addWidget(siteSpreadSheet,0,0);
    siteSpreadSheet->setColumnCount(7);
    siteSpreadSheet->setRowCount(1);
    QStringList tableHeadings;
    tableHeadings << "Station" << "Latitude" << "Longitude" << "Vs30 (m/s)" << "z1pt0 (km)" << "z2pt5 (km)" << "zTR (m)";
    siteSpreadSheet->setHorizontalHeaderLabels(tableHeadings);
    previewGroup->setMaximumHeight(300);

    // Default headers in input csv
    defaultCSVHeader << "Station" << "Latitude" << "Longitude" << "Vs30" << "z1pt0" << "z2pt5" << "zTR";
    for (int i = 0; i != defaultCSVHeader.length(); ++i)
        attributeIndex.insert(defaultCSVHeader[i], i);

    QHBoxLayout* fileNameLayout = new QHBoxLayout();

    fileNameLayout->addWidget(filenameLabel,0,Qt::AlignLeft);
    fileNameLayout->addWidget(FilenameLineEdit,1);
    fileNameLayout->addWidget(browseFileButton,0,Qt::AlignRight);

    QGridLayout* labelsLayout = new QGridLayout();
    labelsLayout->addWidget(minIDLabel,0,0);
    labelsLayout->addWidget(minIDLineEdit,0,1);
    labelsLayout->addWidget(maxIDLabel,1,0);
    labelsLayout->addWidget(maxIDLineEdit,1,1);

    layout->addLayout(fileNameLayout,0,0,1,2);

    layout->addWidget(previewGroup,1,0);
    layout->addLayout(labelsLayout,1,1,1,1,Qt::AlignRight);


    //Now we need to setup the connections
    setupConnections();
}


void SiteScatterWidget::setupConnections()
{

    connect(this->minIDLineEdit, &QLineEdit::textChanged,
            this, &SiteScatterWidget::setMinID);

    connect(this->maxIDLineEdit, &QLineEdit::textChanged,
            this, &SiteScatterWidget::setMaxID);

    connect(browseFileButton,
            &QPushButton::clicked,
            this,
            &SiteScatterWidget::loadSiteFile);

}


void SiteScatterWidget::setMinID()
{
    if (minIDLineEdit->text().isEmpty())
    {
        QString errMsg = "Please provide Min ID.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        minID = minIDLineEdit->text().toInt();
    }
}


void SiteScatterWidget::setMaxID()
{
    if (maxIDLineEdit->text().isEmpty())
    {
        QString errMsg = "Please provide Max ID.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        maxID = maxIDLineEdit->text().toInt();
    }
}


void SiteScatterWidget::setSiteFile(QString dirPath)
{
    this->FilenameLineEdit->setText(dirPath);
    return;
}


void SiteScatterWidget::loadSiteFile()
{
    this->siteFilePath=QFileDialog::getOpenFileName(this,tr("Site File"));
    if(this->siteFilePath.isEmpty())
    {
        this->siteFilePath = "NULL";
        return;
    }
    this->setSiteFile(this->siteFilePath);

    //Connecting the filename
    if (this->siteFilePath.compare("Null") == 0 && this->siteFilePath.contains(".csv", Qt::CaseInsensitive))
    {
        QString errMsg = "Please choose a Site File (.csv)";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        fileLoaded = true;
    }

    //Loading the file
    auto loadFlag = this->parseSiteFile(this->siteFilePath);
    if (loadFlag != 0)
    {
        QString errMsg = "File: " + this->siteFilePath + " cannot be parsed.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
        this->statusMessage("Site file loaded.");
}


int SiteScatterWidget::parseSiteFile(const QString& pathToFile)
{
    // tmpData is a QVector with QStringLists
    // Each QStringList is for one record
    // With divider ",", QStringList has multiple attributes
    auto tmpData = parseCSVFile(pathToFile);

    // Empty check
    if (tmpData.empty())
        return -1;

    auto colName = tmpData[0];
    // Find the required column names and their corresponding indices
    QMap<QString, int>::iterator i;
    for (i = attributeIndex.begin(); i != attributeIndex.end(); ++i)
    {
        for (int j = 0; j != colName.size(); ++j)
        {
            if (i.key().compare(colName[j]) == 0)
                attributeIndex[i.key()] = j;
        }
    }

    // Parse site data
    m_siteScatter.clearSites();
    m_siteScatter.initialize(tmpData.length());
    for (int stag = 1; stag != tmpData.length(); stag++)
    {
        auto curSite = tmpData[stag];
        UserSpecifiedSite site;

        // Required fields
        if (colName.contains("Station"))
        {
            site.SiteNum = curSite[attributeIndex["Station"]].toInt();
        }
        else
        {
            QString errMsg = "Please include the \"Station\" in the site file.";
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return 1;
        }
        if (colName.contains("Latitude"))
        {
            site.Latitude = curSite[attributeIndex["Latitude"]];
        }
        else
        {
            QString errMsg = "Please include the \"Latitude\" in the site file.";
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return 1;
        }
        if (colName.contains("Longitude"))
        {
            site.Longitude = curSite[attributeIndex["Longitude"]];
        }
        else
        {
            QString errMsg = "Please include the \"Longitude\" in the site file.";
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return 1;
        }

        // Optional fields (can be extended in future)
        if (colName.contains("Vs30"))
            site.Vs30 = curSite[attributeIndex["Vs30"]];
        else
            site.Vs30 = "";
        if (colName.contains("z1pt0"))
            site.z1pt0 = curSite[attributeIndex["z1pt0"]];
        else
            site.z1pt0 = "";
        if (colName.contains("z2pt5"))
            site.z2pt5 = curSite[attributeIndex["z2pt5"]];
        else
            site.z2pt5 = "";
        if (colName.contains("zTR"))
            site.zTR = curSite[attributeIndex["zTR"]];
        else
            site.zTR = "";

        m_siteScatter.addSite(site);
    }

    this->statusMessage("Site file parsed.");

    // update the table
    tmpData.pop_front();
    this->updateSiteSpreadSheet(m_siteScatter.getSiteList());

    return 0;
}


QVector<QStringList> SiteScatterWidget::parseCSVFile(const QString &pathToFile)
{
    QVector<QStringList> returnVec;

    QFile tmpFile(pathToFile);

    if (!tmpFile.open(QIODevice::ReadOnly))
    {
        QString errMsg = "Cannot find the file: " + pathToFile + "\nCheck your directory and try again.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return returnVec;
    }

    QStringList rowLines;
    while (!tmpFile.atEnd())
    {
        QString line = tmpFile.readLine();

        rowLines << line;
    }

    auto numRows = rowLines.size();
    if(numRows == 0)
    {
        QString errMsg = "Error in parsing the .csv file "+pathToFile;
        qDebug()<<errMsg;
        this->errorMessage(errMsg);
        return returnVec;
    }

    returnVec.reserve(numRows);

    for(auto&& it: rowLines)
    {
        auto lineStr = this->parseLineCSV(it);

        returnVec.push_back(lineStr);
    }

    return returnVec;
}


QStringList SiteScatterWidget::parseLineCSV(const QString &csvString)
{
    QStringList fields;
    QString value;

    bool hasQuote = false;

    for (int i = 0; i < csvString.size(); ++i)
    {
        const QChar current = csvString.at(i);

        // Normal state
        if (hasQuote == false)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                hasQuote = true;
                value += current;
            }

            // Other character
            else
                value += current;
        }
        else if (hasQuote)
        {
            // Check for another double-quote
            if (current == '"')
            {
                if (i < csvString.size())
                {
                    // A double double-quote?
                    if (i+1 < csvString.size() && csvString.at(i+1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        hasQuote = false;
                        value += '"';
                    }
                }
            }

            // Other character
            else
                value += current;
        }
    }

    if (!value.isEmpty())
        fields.append(value.trimmed());


    // Remove quotes and whitespace around quotes
    for (int i=0; i<fields.size(); ++i)
        if (fields[i].length()>=1 && fields[i].left(1)=='"')
        {
            fields[i]=fields[i].mid(1);
            if (fields[i].length()>=1 && fields[i].right(1)=='"')
                fields[i]=fields[i].left(fields[i].length()-1);
        }

    return fields;
}


void SiteScatterWidget::updateSiteSpreadSheet(const QList<UserSpecifiedSite>& siteList)
{

    if(siteList.empty())
    {
        qDebug()<<"Error: Empty data.";
        return;
    }

    siteSpreadSheet->clear();
    siteSpreadSheet->setColumnCount(attributeIndex.keys().length());
    siteSpreadSheet->setRowCount(siteList.size());
    QStringList tableHeadings;
    foreach (const QString str, defaultCSVHeader)
        tableHeadings << str;
    siteSpreadSheet->setHorizontalHeaderLabels(tableHeadings);
    //flatfileSpreadSheet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    siteSpreadSheet->verticalHeader()->setVisible(false);

    updatingSiteTable = true;
    // Preview 100 sites at most
    int preview_size;
    if (siteList.size() > 100)
        preview_size = 100;
    else
        preview_size = siteList.size();
    for (int i = 0; i< preview_size; ++i)
    {
        QList<QString> tableRow;
        tableRow << QString::number(siteList[i].SiteNum) << siteList[i].Latitude << siteList[i].Longitude
                 << siteList[i].Vs30 << siteList[i].z1pt0 << siteList[i].z2pt5 << siteList[i].zTR;

        QTableWidgetItem *item;
        for (int j = 0; j != defaultCSVHeader.length(); ++j)
        {
            item = new QTableWidgetItem(tableRow[j]);
            siteSpreadSheet->setItem(i, j, item);
        }
    }

    updatingSiteTable = false;

    return;
}


bool SiteScatterWidget::getFileLoaded() const
{
    return fileLoaded;
}


bool SiteScatterWidget::siteFileExists()
{
    QString filename = this->siteFilePath;

    QFile fileToCopy(filename);

    if (! fileToCopy.exists())
        return false;

    return true;
}


bool SiteScatterWidget::copySiteFile(const QString& destinationDir)
{

    QDir dirInput(destinationDir);
    if (!dirInput.exists())
    {
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->errorMessage(errMsg);
            return false;
        }
    }

    QString filename = this->siteFilePath;

    if(!this->siteFileExists())
    {
        QString errMsg = "Cannot find the site file." + QString(filename);
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return false;
    }

    QFile fileToCopy(filename);

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + "SiteFile.csv");
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing site file in the input directory.";
        qDebug() << warnMsg;
        this->infoMessage(warnMsg);
    }

    // Copy
    return fileToCopy.copy(destinationDir + QDir::separator() + "SiteFile.csv");
}


int SiteScatterWidget::getMinID()
{
    return minID;
}


int SiteScatterWidget::getMaxID()
{
    return maxID;
}

