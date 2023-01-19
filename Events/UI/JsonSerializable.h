#ifndef JSONSERIALIZABLE_H
#define JSONSERIALIZABLE_H
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

// This class is used when we need to have Json input/output functionality but we cannot derive from a class with a QWidget baseclass

#include <QJsonObject>
#include <Utils/PythonProgressDialog.h>

class JsonSerializable
{
public:
    virtual bool outputToJSON(QJsonObject &jsonObject) = 0;

    virtual bool inputFromJSON(QJsonObject &jsonObject) = 0;

    virtual void reset(void) = 0;

#ifdef OpenSRA
   inline void errorMessage(const QString& message)
   {
       if(message.isEmpty())
           return;

       PythonProgressDialog::getInstance()->appendErrorMessage(message);
   }

   inline void statusMessage(const QString& message)
   {
       if(message.isEmpty())
           return;

       PythonProgressDialog::getInstance()->appendText(message);
   }
#endif

   // Get and set the json obj for this parameter
   inline const QJsonObject& getMethodAndParamJsonObj() const
   {
       return jsonObj;
   }


   inline void setMethodAndParamJsonObj(const QJsonObject &newJsonObj)
   {
       jsonObj = newJsonObj;
   }

protected:

   QJsonObject jsonObj;
};

#endif // JSONSERIALIZABLE_H
