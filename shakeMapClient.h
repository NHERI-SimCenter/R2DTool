#ifndef SHAKEMAPCLIENT_H
#define SHAKEMAPCLIENT_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

class shakeMapClient : public QObject
{
    Q_OBJECT
public:
    shakeMapClient(QObject *parent = nullptr);


private slots:
    void processNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager networkManager;

    QNetworkReply* usgsReply;

    QNetworkRequest uploadFileRequest;

    void processUSGSReply();

};

#endif // SHAKEMAPCLIENT_H
