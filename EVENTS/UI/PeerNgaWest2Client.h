#ifndef PEERNGAWEST2CLIENT_H
#define PEERNGAWEST2CLIENT_H
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

// Written by: Frank McKenna

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QStringList>

class PeerNgaWest2Client : public QObject
{
    Q_OBJECT
public:
    explicit PeerNgaWest2Client(QObject *parent = nullptr);
    bool loggedIn();
    void signIn(QString username, QString password);
    void selectRecords(double sds, double sd1, double tl, int nRecords, QVariant magnitudeRange, QVariant distanceRange, QVariant vs30Range);
    void selectRecords(QList<QPair<double, double>> spectrum, int nRecords, QVariant magnitudeRange, QVariant distanceRange, QVariant vs30Range);
    void selectRecords(QStringList);

    void setScalingParameters(const int scaleFlag,
                              const QString& periodPoints,
                              const QString& weightPoints,
                              const QString& scalingPeriod);

signals:
    void loginFinished(bool result);
    void recordsDownloaded(QString recordsPath);
    void statusUpdated(QString status);
    void selectionStarted();
    void selectionFinished();

public slots:

private:
    QNetworkAccessManager networkManager;

    QNetworkReply* signInPageReply;
    QNetworkReply* signInReply;
    QNetworkReply* getSpectraReply;
    QNetworkReply* postSpectraReply;
    QNetworkReply* postSearchReply;
    QNetworkReply* getRecordsReply;
    QNetworkReply* downloadRecordsReply;
    QNetworkReply* uploadFileReply;

    QString authenticityToken;
    QString username;
    QString password;
    int nRecords;
    bool isLoggedIn;
    QVariant magnitudeRange;
    QVariant distanceRange;
    QVariant vs30Range;

    int searchScaleFlag;
    QString searchPeriodPoints;
    QString searchWeightPoints;
    QString searchSinglePeriodScalingT;

    //Data for retry on failure
    int retries;
    QList<QNetworkCookie> signInCookies;
    QNetworkRequest postSpectraRequest;
    QUrlQuery postSpectraParameters;
    QNetworkRequest peerSignInRequest;
    QUrlQuery signInParameters;
    QNetworkRequest uploadFileRequest;
    QStringList recordsToDownload;

    void setupConnection();
    void processNetworkReply(QNetworkReply *reply);

    void processSignInPageReply();
    void processSignInReply();
    void processUploadFileReply();
    void processPostSpectrumReply();
    void processPostSearchReply();
    void processGetRecordsReply();
    void processDownloadRecordsReply();
    void retryPostSpectra();
    void retrySignIn();
    void retry();


};

#endif // PEERNGAWEST2CLIENT_H
