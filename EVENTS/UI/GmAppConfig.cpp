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

/*
    if(username.isEmpty() || password.isEmpty())
    {
        err = "Check that you have input your username and password for access to the record selection databse";

        return false;
    }

*/
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
