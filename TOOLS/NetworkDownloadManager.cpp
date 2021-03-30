#include "NetworkDownloadManager.h"

NetworkDownloadManager::NetworkDownloadManager()
{
    connect(&manager, &QNetworkAccessManager::finished,
            this, &NetworkDownloadManager::downloadFinished);

    numRedirects = 0;
}


void NetworkDownloadManager::downloadSingle(const QUrl &url, const QString& fileName)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    reply->setProperty("FileName",fileName);

#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkDownloadManager::sslErrors);
#endif

    currentDownloads.append(reply);
}



bool NetworkDownloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug()<< "Could not open %s for writing: %s\n" <<
                   qPrintable(filename) <<
                   qPrintable(file.errorString());

        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}


bool NetworkDownloadManager::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308;
}


void NetworkDownloadManager::executeDownloads(const QStringList urls, const QStringList fileNames)
{
    if (urls.isEmpty() || urls.size() != fileNames.size())
    {
        qDebug()<<"The download list is empty";
        return;
    }

    for(int i = 0; i <urls.size(); ++i)
    {
        auto urlStr = urls.at(i);
        auto fileName = fileNames.at(i);

        QUrl url = QUrl::fromEncoded(urlStr.toLocal8Bit());
        downloadSingle(url,fileName);
    }
}


void NetworkDownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}


QUrl NetworkDownloadManager::redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const
{
    QUrl redirectUrl;

    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl)
    {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}

void NetworkDownloadManager::downloadFinished(QNetworkReply *reply)
{

    auto fNameVariant = reply->property("FileName");

    auto fileName  = fNameVariant.toString();

    if(fileName.isEmpty())
    {
        qDebug()<<"Failed to get file name";
        return;
    }

    QUrl url = reply->url();
    if (reply->error())
    {
        qCritical() << "Download of "<<fileName<<" failed: \n" <<
                       url.toEncoded().constData() <<
                       qPrintable(reply->errorString());
    }
    else
    {
        if (isHttpRedirect(reply))
        {
            ++numRedirects;

            if(numRedirects > 3)
            {
                qCritical()<<"Exceeded the number of allowable redirects";
                return;
            }

            // Ask for the reply about the Redirection attribute
            QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

            // Deduce if the redirection is valid in the redirectUrl function
            urlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);

            // The URL is not empty, we're being redirected
            if(!urlRedirectedTo.isEmpty())
            {
                // Do another request to the redirection url
                this->downloadSingle(urlRedirectedTo,fileName);
            }
        }
        else
        {
            auto pathExamplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples";

            auto pathToSaveFile = pathExamplesFolder + QDir::separator() + fNameVariant.toString();

            QDir examplesDir(pathExamplesFolder);

            if(!examplesDir.exists())
                examplesDir.mkdir(".");

            if (saveToDisk(pathToSaveFile, reply))
            {
                qDebug()<<"Download of succeeded "<<fNameVariant<<" \n";
            }

            // Arrived at the final URL
            urlRedirectedTo.clear();
            numRedirects = 0;
        }
    }

    // Clean up
    currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (currentDownloads.isEmpty())
    {
        // all downloads finished
        qDebug()<<"All downloads are finished";
    }
}


