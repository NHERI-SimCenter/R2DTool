#include "GmAppConfig.h"

#include <QFile>
#include <QDir>
#include <QJsonObject>

GmAppConfig::GmAppConfig(QObject *parent) : QObject(parent)
{

}


QString GmAppConfig::getWorkDirectoryPath() const
{
    return workDirectoryPath;
}


void GmAppConfig::setWorkDirectoryPath(const QString &value)
{
    if(workDirectoryPath != value)
    {
        workDirectoryPath = value;
        emit workingDirectoryPathChanged(workDirectoryPath);
    }
}


QString GmAppConfig::getInputDirectoryPath() const
{
    return inputDirectoryPath;
}


void GmAppConfig::setInputFilePath(const QString &value)
{
    if(inputDirectoryPath != value)
    {
        inputDirectoryPath = value;
        emit inputDirectoryPathChanged(inputDirectoryPath);
    }
}


QString GmAppConfig::getOutputDirectoryPath() const
{
    return outputDirectoryPath;
}


void GmAppConfig::setOutputFilePath(const QString &value)
{
    if(outputDirectoryPath != value)
    {
        outputDirectoryPath = value;
        emit outputDirectoryPathChanged(outputDirectoryPath);
    }
}


QString GmAppConfig::getUsername() const
{
    return username;
}


void GmAppConfig::setUsername(const QString &value)
{
    if(username != value)
    {
        username = value;
        emit usernamePathChanged(username);
    }
}


QString GmAppConfig::getPassword() const
{
    return password;
}


bool GmAppConfig::validate(QString& err)
{

    if(!QDir(workDirectoryPath).exists())
    {
        // Try to make the directory
        auto res = QDir().mkdir(workDirectoryPath);

        if(res == false)
        {
            err = "Error: Cannot find or make the directory "+workDirectoryPath;

            return false;
        }
    }


    if(!QDir(inputDirectoryPath).exists())
    {
        // Try to make the directory
        auto res = QDir().mkdir(inputDirectoryPath);

        if(res == false)
        {
            err = "Error: Cannot find or make the directory "+inputDirectoryPath;

            return false;
        }
    }


    if(!QDir(outputDirectoryPath).exists())
    {
        // Try to make the directory
        auto res = QDir().mkdir(outputDirectoryPath);

        if(res == false)
        {
            err = "Error: Cannot find or make the directory "+outputDirectoryPath;

            return false;
        }
    }


    if(username.isEmpty() || password.isEmpty())
    {
        err = "Check that you have input your username and password for access to the record selection databse";

        return false;
    }


    return true;
}


void GmAppConfig::setPassword(const QString &value)
{
    if(password != value)
    {
        password = value;
        emit passwordChanged(password);
    }
}


QJsonObject GmAppConfig::getJson()
{
    QJsonObject jsonObj;

    jsonObj.insert("Work",workDirectoryPath);
    jsonObj.insert("Input",inputDirectoryPath);
    jsonObj.insert("Output",outputDirectoryPath);

    return jsonObj;
}
