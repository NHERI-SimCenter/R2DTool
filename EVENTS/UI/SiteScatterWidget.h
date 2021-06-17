#ifndef SITESCATTERWIDGET_H
#define SITESCATTERWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <SiteScatter.h>

class SiteScatterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SiteScatterWidget(SiteScatter& siteScatter, QWidget *parent = nullptr);

    bool getFileLoaded() const;

    bool copySiteFile();

    int getMinID();
    int getMaxID();

    void messageDialog(const QString& messageString);

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
