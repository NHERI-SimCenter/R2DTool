#ifndef LOCALITY_PYRECODES_H
#define LOCALITY_PYRECODES_H

/* *****************************************************************************
Copyright (c) 2016-2023, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SSensorsTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES Sensors MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPBodiesE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT Sensors SUBSTITUTE GOODS OR SERVICES;
LBodiesS Sensors USE, DATA, OR PRSensorsITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY Sensors LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT Sensors THE USE Sensors THIS
SSensorsTWARE, EVEN IF ADVISED Sensors THE PBodiesSIBILITY Sensors SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES Sensors MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPBodiesE.
THE SSensorsTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

/**
 *  @author  fmk
 *  @date    1/2024
 *  @version 4.0
 *
 *  @section DESCRIPTION
 *
 *  This is the class providing the Outputs Tab for the MPM application
 */

#include <SimCenterWidget.h>

class  SC_FileEdit;
class  QLineEdit;
class  SC_CheckBox;
class  QListWidget;
class PyrecodesSystemConfig;


class PyrecodesLocality : public SimCenterWidget
{
    Q_OBJECT
public:
  
  PyrecodesLocality(PyrecodesSystemConfig *theConfig, QString theName, QWidget *parent = 0);
  virtual ~PyrecodesLocality();
  bool outputToJSON(QJsonObject &jsonObject);
  bool inputFromJSON(QJsonObject &jsonObject);
  bool copyFiles(QString &dirName);
  void clear(void);
  
signals:

private:

  // name
  QLineEdit  *theName;
  
  // coordinates
  SC_FileEdit        *coordinateFile;
  QLineEdit  *boundingBox;

  // components
  SC_CheckBox *buildingsCB;
  SC_CheckBox *infrastructureCB;  
  //SC_CheckBox *waterCB;
  //SC_CheckBox *transportationCB;
  QListWidget *infrastructureList;
  SC_CheckBox *resourceSupplierCB;

  PyrecodesSystemConfig *theSystemConfig;
};

#endif // OUTPUTS_DIGITAL_TWIN_H
