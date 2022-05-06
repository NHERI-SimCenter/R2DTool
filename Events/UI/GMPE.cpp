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

#include "GMPE.h"

GMPE::GMPE(QObject *parent) : QObject(parent)
{
    this->m_type = "Chiou & Youngs (2014)";
}


QString GMPE::type() const
{
    return m_type;
}


bool GMPE::setType(const QString &type)
{
    if(m_type!= type && this->validTypes().contains(type, Qt::CaseInsensitive))
    {
        m_type = type;
        emit typeChanged(m_type);
        return true;
    }

    return false;
}


const QStringList &GMPE::validTypes()
{
    static QStringList validTypes = QStringList()
            << "Abrahamson, Silva & Kamai (2014)"
            << "Boore, Stewart, Seyhan & Atkinson (2014)"
            << "Campbell & Bozorgnia (2014)"
            << "Chiou & Youngs (2014)";
//            << "Idriss (2014)"
//            << "Campbell & Bozorgnia (2008)"
//            << "Boore & Atkinson (2008)"
//            << "Abrahamson & Silva (2008)"
//            << "Chiou & Youngs (2008)"
//            << "Zhao Et Al. (2006) - Intraslab"
//            << "Boore & Atkinson (2006)"
//            << "Chiou & Youngs (2006)"
//            << "Campbell & Bozorgnia (2006)"
//            << "Campbell and Bozorgnia (2003)"
//            << "Atkinson and Boore (2003) Subduction"
//            << "Field (2000)"
//            << "Campbell (1997) w/ erratum (2000) changes"
//            << "Abrahamson & Silva (1997)"
//            << "Boore, Joyner & Fumal (1997)";

    return validTypes;
}

bool GMPE::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type", m_type);
    jsonObject.insert("Parameters", QJsonObject());

    return true;
}


bool GMPE::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


void GMPE::reset(void)
{
    this->m_type = "Chiou & Youngs (2014)";
}

