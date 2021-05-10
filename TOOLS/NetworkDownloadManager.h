#ifndef NETWORKDOWNLOADMANAGER_H
#define NETWORKDOWNLOADMANAGER_H
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

// Written by: Stevan Gavrilovic

#include <SimCenterWidget.h>

#include <QtNetwork>

class QSslError;

using namespace std;

class NetworkDownloadManager: public SimCenterWidget
{
    Q_OBJECT

public:
    NetworkDownloadManager(QWidget *parent);

    void downloadSingleFile(const QUrl &url, const QString& fileName, const QString& fileHash);

    void downloadSingleFileInfo(const QUrl &url, const QString& fileName);

    bool saveToDisk(const QString &filename, QIODevice *data, const QString &remoteHash);

    static bool isHttpRedirect(QNetworkReply *reply);

    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;

signals:

    void downloadSuccess(bool result);

public slots:
    // Downloads the file info
    void downloadExamples(const QStringList urls, const QStringList fileNames);

    // Slot to let the program know that the download is finished
    void fileDownloadFinished(QNetworkReply *reply);

    // Slot to let the program know that the information is downloaded
    void fileInfoDownloadFinished(QNetworkReply *reply);

    // Slot to display any SSL errors
    void sslErrors(const QList<QSslError> &errors);

    // Returns empty QByteArray() on failure
    QString getFileMD5Checksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);

private slots:
    // Downloads the given urls in parallel
    void downloadAllFiles();

private:
    QNetworkAccessManager fileDownloadManager;
    QNetworkAccessManager infoDownloadManager;

    QList<QNetworkReply *> currentFileDownloads;
    QList<QNetworkReply *> currentInfoDownloads;

    QUrl FileDownloadUrlRedirectedTo;
    size_t FileDownloadNumRedirects;

    QUrl FileInfoUrlRedirectedTo;
    size_t FileInfoNumRedirects;

    QStringList fileDownloadUrls;
    QStringList fileDownloadNames;
    QStringList fileHashes;

    int unzipFile(const QString pathZipFile);

    void cleanup(void);
};


#endif // NETWORKDOWNLOADMANAGER_H
