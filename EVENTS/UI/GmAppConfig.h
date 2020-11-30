#ifndef GMAPPCONFIG_H
#define GMAPPCONFIG_H

#include <QObject>

class GmAppConfig : public QObject
{
    Q_OBJECT
public:
    explicit GmAppConfig(QObject *parent = nullptr);

    QString getWorkDirectoryPath() const;
    QString getInputDirectoryPath() const;
    QString getOutputDirectoryPath() const;

    QString getUsername() const;
    QString getPassword() const;

    bool validate(QString& err);

    QJsonObject getJson();

signals:
    void workingDirectoryPathChanged(QString newPath);
    void inputDirectoryPathChanged(QString newPath);
    void outputDirectoryPathChanged(QString newPath);

    void usernamePathChanged(QString newPath);
    void passwordChanged(QString newPath);

public slots:
    void setWorkDirectoryPath(const QString &value);
    void setInputFilePath(const QString &value);
    void setOutputFilePath(const QString &value);
    void setUsername(const QString &value);
    void setPassword(const QString &value);

private:
    QString workDirectoryPath;
    QString inputDirectoryPath;
    QString outputDirectoryPath;

    QString username;
    QString password;
};

#endif // GMAPPCONFIG_H
