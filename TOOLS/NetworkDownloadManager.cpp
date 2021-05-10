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

#include "NetworkDownloadManager.h"
#include "ZipUtils.h"
#include "Utils/PythonProgressDialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressBar>

NetworkDownloadManager::NetworkDownloadManager(QWidget *parent) : SimCenterWidget(parent)
{
    connect(&fileDownloadManager, &QNetworkAccessManager::finished,
            this, &NetworkDownloadManager::fileDownloadFinished);

    connect(&infoDownloadManager, &QNetworkAccessManager::finished,
            this, &NetworkDownloadManager::fileInfoDownloadFinished);


    FileDownloadNumRedirects = 0;
    FileInfoNumRedirects = 0;
}


void NetworkDownloadManager::downloadSingleFile(const QUrl &url, const QString& fileName, const QString& fileHash)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = fileDownloadManager.get(request);

    reply->setProperty("FileName",fileName);
    reply->setProperty("FileHash",fileHash);


#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkDownloadManager::sslErrors);
#endif

    currentFileDownloads.append(reply);
}


void  NetworkDownloadManager::downloadSingleFileInfo(const QUrl &url, const QString& fileName)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = infoDownloadManager.get(request);

    reply->setProperty("FileName",fileName);

#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkDownloadManager::sslErrors);
#endif

    currentInfoDownloads.append(reply);
}



bool NetworkDownloadManager::saveToDisk(const QString &filename, QIODevice *data, const QString &remoteHash)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QString err = "Could not open %s for writing: %s\n" + filename + file.errorString();
        errorMessage(err);

        return false;
    }

    file.write(data->readAll());
    file.close();

    // Check the hash to ensure the file has been downloaded
    auto localHash = this->getFileMD5Checksum(filename,QCryptographicHash::Md5);

    if(localHash.compare(remoteHash) != 0)
    {
        QString err = "Hash failed, try to download the file "+filename+" again";
        errorMessage(err);
        return false;
    }


    // Now unzip the file
    auto res = unzipFile(filename);
    if(res != 0)
    {
        QString err = "Failed to unzip the file " + filename;
        errorMessage(err);
        return false;
    }

    // Remove the zip file
    QFile zipFile (filename);
    zipFile.remove();

    return true;
}


int NetworkDownloadManager::unzipFile(const QString pathZipFile)
{
    auto pathExamplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples";

    auto pathToOutputDirectory = pathExamplesFolder + QDir::separator();

    bool result =  ZipUtils::UnzipFile(pathZipFile, pathToOutputDirectory);
    if (result == false)
    {
        QString err = "Error in unziping the downloaded example files";
        errorMessage(err);
        return -1;
    }

    return 0;
}


bool NetworkDownloadManager::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308;
}


void NetworkDownloadManager::downloadAllFiles(void)
{
    if (fileDownloadUrls.isEmpty() || fileDownloadUrls.size() != fileDownloadNames.size() || fileHashes.size()  != fileDownloadNames.size())
    {
        QString err ="The download list is empty";
        errorMessage(err);
        return;
    }

    QString msg = "Starting download of "+ QString::number(fileDownloadUrls.size()) + " examples ";
    statusMessage(msg);

    this->getProgressDialog()->showProgressBar();
    this->getProgressDialog()->setProgressBarRange(0, fileDownloadUrls.size());

    for(int i = 0; i <fileDownloadUrls.size(); ++i)
    {
        auto urlStr = fileDownloadUrls.at(i);
        auto fileName = fileDownloadNames.at(i);
        auto fileHash = fileHashes.at(i);

        QUrl url = QUrl::fromEncoded(urlStr.toLocal8Bit());

        QString msg = "Downloading example "+fileName;
        statusMessage(msg);

        downloadSingleFile(url,fileName,fileHash);
    }
}


void NetworkDownloadManager::downloadExamples(const QStringList urls, const QStringList fileNames)
{
    if (urls.isEmpty() || urls.size() != fileNames.size())
    {
        QString err ="The download list is empty";
        errorMessage(err);
        return;
    }

    for(int i = 0; i <urls.size(); ++i)
    {
        auto urlStr = urls.at(i);
        QUrl url = QUrl::fromEncoded(urlStr.toLocal8Bit());

        auto fileName = fileNames.at(i);

        this->downloadSingleFileInfo(url,fileName);
    }
}


void NetworkDownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
    {
        QString err = QString(fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString())));
        errorMessage(err);
    }
#else
    Q_UNUSED(sslErrors);
#endif
}


QUrl NetworkDownloadManager::redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const
{
    QUrl redirectUrl;

    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl)
        redirectUrl = possibleRedirectUrl;

    return redirectUrl;
}


void NetworkDownloadManager::fileDownloadFinished(QNetworkReply *reply)
{
    auto fNameVariant = reply->property("FileName");

    auto fileName  = fNameVariant.toString();

    if(fileName.isEmpty())
    {
        QString err = "Failed to get file name from the network reply";
        errorMessage(err);
        emit downloadSuccess(false);
        reply->deleteLater();
        this->cleanup();
        return;
    }


    auto fHashVariant = reply->property("FileHash");

    auto fileHash  = fHashVariant.toString();

    if(fileHash.isEmpty())
    {
        QString err = "Failed to get file hash";
        errorMessage(err);
        emit downloadSuccess(false);
        reply->deleteLater();
        this->cleanup();
        return;
    }


    QUrl url = reply->url();
    if (reply->error())
    {
        QString err = "Download of "+ fileName +" failed: \n" + url.toEncoded().constData() + reply->errorString();
        errorMessage(err);
        emit downloadSuccess(false);
        reply->deleteLater();
        this->cleanup();
        return;
    }
    else
    {
        if (isHttpRedirect(reply))
        {
            ++FileDownloadNumRedirects;

            if(FileDownloadNumRedirects > 5)
            {
                QString err = "Exceeded the number of allowable redirects for "+fileName;
                errorMessage(err);
                reply->deleteLater();
                this->cleanup();
                return;
            }

            // Ask for the reply about the Redirection attribute
            QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

            // Deduce if the redirection is valid in the redirectUrl function
            FileDownloadUrlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl(), FileDownloadUrlRedirectedTo);

            // The URL is not empty, we're being redirected
            if(!FileDownloadUrlRedirectedTo.isEmpty())
            {
                // Do another request to the redirection url
                this->downloadSingleFile(FileDownloadUrlRedirectedTo,fileName, fileHash);
            }
        }
        else
        {
            auto pathExamplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples";

            auto pathToSaveFile = pathExamplesFolder + QDir::separator() + fNameVariant.toString() + ".zip";

            QDir examplesDir(pathExamplesFolder);

            if(!examplesDir.exists())
                examplesDir.mkdir(".");

            if (saveToDisk(pathToSaveFile, reply,fileHash))
            {
                QString msg = "Installation succeeded of  "+ fNameVariant.toString();
                statusMessage(msg);
            }
            else
            {
                reply->deleteLater();
                this->cleanup();
                return;
            }

            // Arrived at the final URL
            FileDownloadUrlRedirectedTo.clear();
            FileDownloadNumRedirects = 0;
        }
    }

    // Clean up
    currentFileDownloads.removeAll(reply);
    reply->deleteLater();

    auto upperRange = this->getProgressDialog()->getProgressBar()->maximum();
    this->getProgressDialog()->setProgressBarValue(upperRange-currentFileDownloads.size());

    if (currentFileDownloads.isEmpty())
    {
        // all downloads finished
        QString msg = "All example downloads are finished.  Go to the 'Examples' menu to load an example.";
        statusMessage(msg);
        emit downloadSuccess(true);
        this->cleanup();
    }
}


void NetworkDownloadManager::fileInfoDownloadFinished(QNetworkReply *reply)
{
    auto fNameVariant = reply->property("FileName");

    auto fileName  = fNameVariant.toString();

    if(fileName.isEmpty())
    {
        QString err = "Failed to get file name from the reply";
        errorMessage(err);
        emit downloadSuccess(false);
        reply->deleteLater();
        this->cleanup();
        return;
    }

    QUrl url = reply->url();
    if (reply->error())
    {
        QString err = "Download failed: \n" + url.toString() + reply->errorString();
        errorMessage(err);
        emit downloadSuccess(false);
        reply->deleteLater();
        this->cleanup();
        return;
    }
    else
    {
        if (isHttpRedirect(reply))
        {
            ++FileInfoNumRedirects;

            if(FileInfoNumRedirects > 5)
            {
                QString err = "Exceeded the number of allowable redirects for getting the info for "+fileName;
                errorMessage(err);
                emit downloadSuccess(false);
                reply->deleteLater();
                this->cleanup();
                return;
            }

            // Ask for the reply about the Redirection attribute
            QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

            // Deduce if the redirection is valid in the redirectUrl function
            FileInfoUrlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl(), FileInfoUrlRedirectedTo);

            // The URL is not empty, we're being redirected
            if(!FileInfoUrlRedirectedTo.isEmpty())
            {
                // Do another request to the redirection url
                this->downloadSingleFileInfo(FileInfoUrlRedirectedTo,fileName);
            }
        }
        else
        {

            auto jsonReply = reply->readAll();

            QJsonDocument doc = QJsonDocument::fromJson(jsonReply);

            auto obj = doc.object();

            if(obj.isEmpty())
            {
                QString err = "Empty information object received for "+fileName;
                errorMessage(err);
                emit downloadSuccess(false);
                reply->deleteLater();
                this->cleanup();
                return;
            }

            auto downloadFile = obj["files"].toArray();

            if(downloadFile.isEmpty())
            {
                QString err = "Could not get the download URL for "+fileName;
                errorMessage(err);
                emit downloadSuccess(false);
                reply->deleteLater();
                this->cleanup();
                return;
            }

            auto downloadLink = downloadFile.at(0)["links"].toObject()["self"].toString();

            auto fileHash = downloadFile.at(0)["checksum"].toString();

            fileHash.remove("md5:");

            if(!downloadLink.isEmpty() && !fileHash.isEmpty())
            {
                fileDownloadUrls.push_back(downloadLink);
                fileDownloadNames.push_back(fileName);
                fileHashes.push_back(fileHash);
            }
            else
            {
                QString err = "Could not get the download URL";
                errorMessage(err);
                emit downloadSuccess(false);
                reply->deleteLater();
                this->cleanup();
                return;
            }

            // Arrived at the final URL
            FileInfoUrlRedirectedTo.clear();
            FileInfoNumRedirects = 0;
        }
    }

    // Clean up
    currentInfoDownloads.removeAll(reply);
    reply->deleteLater();

    if (currentInfoDownloads.isEmpty())
    {
        this->downloadAllFiles();
    }
}


QString NetworkDownloadManager::getFileMD5Checksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            return QString(hash.result().toHex());
        }
    }

    return QString();
}

void NetworkDownloadManager::cleanup(void)
{
    currentFileDownloads.clear();
    currentInfoDownloads.clear();

    FileDownloadUrlRedirectedTo.clear();
    FileDownloadNumRedirects = 0;

    FileInfoUrlRedirectedTo.clear();
    FileInfoNumRedirects = 0;

    fileDownloadUrls.clear();
    fileDownloadNames.clear();
    fileHashes.clear();

    this->getProgressDialog()->hideProgressBar();
}

