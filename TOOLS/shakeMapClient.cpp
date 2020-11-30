#include "shakeMapClient.h"

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtXml>
#include <QFile>


shakeMapClient::shakeMapClient(QObject *parent) : QObject(parent)
{
    usgsReply = nullptr;

    QNetworkCookie cookie("sourceDb_flag", "1");
    cookie.setDomain("earthquake.usgs.gov");
    networkManager.cookieJar()->insertCookie(cookie);

    QString ScenarioID = "https://earthquake.usgs.gov/fdsnws/scenario/1/query?eventid=";

    QString EventID = "https://earthquake.usgs.gov/fdsnws/event/1/query?eventid=";


    QString sampleEvent = "official19600522191120_30";

    QString test = "https://earthquake.usgs.gov/fdsnws/scenario/1/query?eventid=aklegacyanchorageintraplate_se&amp;format=kml";

    uploadFileRequest.setUrl(QUrl(test));

    QObject::connect(&networkManager, &QNetworkAccessManager::finished, this, &shakeMapClient::processNetworkReply);

    usgsReply = networkManager.get(uploadFileRequest);

}


void shakeMapClient::processNetworkReply(QNetworkReply *reply)
{
    if(reply == usgsReply)
        processUSGSReply();



    else
        qDebug() << reply->readAll();

}


void shakeMapClient::processUSGSReply()
{
    auto replyText = QString(usgsReply->readAll());

    if(replyText.isEmpty())
        return;

//    qDebug()<<replyText;

    // QDomDocument used to import XML data
    QDomDocument xmlGMs;

    // Set raw XML content into the QDomDocument
    xmlGMs.setContent(replyText);

    // Extract the root element
    QDomElement root=xmlGMs.documentElement();

    // Get root names and attributes
    QString Type = root.tagName();

    // Check that the XML file is actually a shake map grid
//    if(Type.compare("shakemap_grid") != 0)
//    {
//        return nullptr;
//    }

}


//{'type': 'origin',
// 'data': {'id': 'us1000abcd',
//          'netid': 'us',
//          'network': '',
//          'time': '2018-05-06T14:12:16.5Z',
//          'lat': '34.5',
//          'lon': '123.6',
//          'depth': '6.2',
//          'mag': '5.6',
//          'locstring': '231 km SE of Guam'
//          'alt_eventids': 'id1,id2,id3',
//          'action': 'Event added'}}
