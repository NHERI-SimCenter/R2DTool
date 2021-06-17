#include "SiteScatter.h"
#include "SiteScatterWidget.h"
#include "HBoxFormLayout.h"
#include "SimCenterPreferences.h"

SiteScatterWidget::SiteScatterWidget(SiteScatter& siteScatter, QWidget *parent) : QWidget(parent), m_siteScatter(siteScatter)
{
    fileLoaded = false;

    //We use a grid layout for the site widget
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Site File box
    QLabel* filenameLabel = new QLabel(tr("Site File (.csv):"),this);
    FilenameLineEdit = new QLineEdit(this);
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

    // Preview of the site file
    auto previewGroup = new QGroupBox(tr("Preview Sites"));
    auto previewLayout = new QGridLayout(previewGroup);
    siteSpreadSheet = new QTableWidget();

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

    layout->addWidget(filenameLabel,0,0);
    layout->addWidget(FilenameLineEdit,0,1,1,6);
    layout->addWidget(browseFileButton,0,7);

    layout->addWidget(minIDLabel,2,6);
    layout->addWidget(minIDLineEdit,2,7,1,2);
    layout->addWidget(maxIDLabel,3,6);
    layout->addWidget(maxIDLineEdit,3,7,1,2);

    layout->addWidget(previewGroup,1,0,3,6);

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
        this->messageDialog(errMsg);
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
        this->messageDialog(errMsg);
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
        this->messageDialog(errMsg);
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
        this->messageDialog(errMsg);
        return;
    }
    else
        this->messageDialog("Site file loaded.");
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
            this->messageDialog(errMsg);
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
            this->messageDialog(errMsg);
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
            this->messageDialog(errMsg);
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

    this->messageDialog("Site file parsed.");

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
        this->messageDialog(errMsg);
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
        qDebug()<<"Error in parsing the .csv file "<<pathToFile<<" in "<<__FUNCSIG__;
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


bool SiteScatterWidget::copySiteFile()
{
    // Destination directory
    QString destinationDir;
    destinationDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + "/HazardSimulation/Input";
    QDir dirInput(destinationDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(destinationDir))
        {
            QString errMsg = QString("Could not make the input directory.");
            qDebug() << errMsg;
            this->messageDialog(errMsg);
            return false;
        }

    QString filename = this->siteFilePath;

    QFile fileToCopy(filename);

    if (! fileToCopy.exists()) {
        QString errMsg = "Cannot find the site file." + QString(filename);
        qDebug() << errMsg;
        this->messageDialog(errMsg);
        return false;
    }

    QFileInfo fileInfo(filename);
    QString theFile = fileInfo.fileName();

    // Overwriting check
    QFile distFile(destinationDir + QDir::separator() + "SiteFile.csv");
    if (distFile.exists())
    {
        distFile.remove();
        QString warnMsg = "Overwriting the existing site file in the input directory.";
        qDebug() << warnMsg;
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


void SiteScatterWidget::messageDialog(const QString& messageString)
{
    if(messageString.isEmpty())
        return;

    QMessageBox msgBox;
    msgBox.setText(messageString);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
}
