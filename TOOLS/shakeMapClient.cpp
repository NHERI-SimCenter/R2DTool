/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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
