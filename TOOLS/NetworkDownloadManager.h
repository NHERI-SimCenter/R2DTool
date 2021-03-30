#ifndef NETWORKDOWNLOADMANAGER_H
#define NETWORKDOWNLOADMANAGER_H

#include <QtNetwork>

class QSslError;

using namespace std;

class NetworkDownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;

public:
    NetworkDownloadManager();

    void downloadSingle(const QUrl &url, const QString& fileName);

    bool saveToDisk(const QString &filename, QIODevice *data);

    static bool isHttpRedirect(QNetworkReply *reply);

    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;

public slots:
    // Downloads the given urls in parallel
    void executeDownloads(const QStringList urls, const QStringList fileNames);

    // Slot to let the program know that the download is finished
    void downloadFinished(QNetworkReply *reply);

    // Slot to display any SSL errors
    void sslErrors(const QList<QSslError> &errors);

private:

    QUrl urlRedirectedTo;
    size_t numRedirects;
};


#endif // NETWORKDOWNLOADMANAGER_H
