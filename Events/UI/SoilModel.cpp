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

// Written by: Kuanshi Zhong

#include "SoilModel.h"
#include <QFileInfo>

SoilModel::SoilModel(QObject *parent) : QObject(parent)
{
    this->m_type = "Elastic Isotropic";
    this->m_userModelPath = "";
}


QString SoilModel::type() const
{
    return m_type;
}


bool SoilModel::setType(const QString &type)
{
    if(m_type!= type && this->validTypes().contains(type, Qt::CaseInsensitive))
    {
        m_type = type;
        emit typeChanged(m_type);
        return true;
    }

    return false;
}


bool SoilModel::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type", m_type);
    if (m_type.compare("User")==0)
    {
        QFileInfo theFile(m_userModelPath);
        jsonObject.insert("Parameters", theFile.fileName());
    }
    else
        jsonObject.insert("Parameters", QJsonObject());

    return true;
}


bool SoilModel::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


const QStringList &SoilModel::validTypesUser()
{
    static QStringList validTypes = QStringList()
            << "Elastic Isotropic"
            << "Multiaxial Cyclic Plasticity"
            << "User";

    return validTypes;
}


const QStringList &SoilModel::validTypes()
{
    static QStringList validTypes = QStringList()
            << "Elastic Isotropic"
            << "Multiaxial Cyclic Plasticity"
            << "User";

    return validTypes;
}


void SoilModel::setUserModelPath(QString userModelPath)
{
    m_userModelPath = userModelPath;
}


QString SoilModel::getUserModelPath()
{
    return m_userModelPath;
}


void SoilModel::reset(void)
{
    this->m_type = "Elastic Isotropic";
    this->m_userModelPath = "";
}
