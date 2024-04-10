#ifndef REWET_RECOVERY_H
#define REWET_RECOVERY_H

/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written: fmk, Sina Naeimi

#include <SimCenterAppWidget.h>


class SC_FileEdit;
class SC_DoubleLineEdit;
class SC_IntLineEdit;
class SC_ComboBox;
class SC_CheckBox;
//class SC_IntLineEdit;
class SC_TableEdit;
class SC_QRadioButton;


class RewetRecovery : public SimCenterAppWidget
{
public:
    RewetRecovery(QWidget *parent = 0);
    ~RewetRecovery();

    bool inputFromJSON(QJsonObject &rvObject);
    bool outputToJSON(QJsonObject &rvObject);  
    bool outputAppDataToJSON(QJsonObject &rvObject);
    bool inputAppDataFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &dirName);

signals:

public slots:
   void clear(void);

private:

  // simulation
  SC_IntLineEdit *simulationTime;
  SC_IntLineEdit *simulationTimeStep;
  SC_CheckBox        *lastTerminationCheckBox;
  SC_CheckBox        *demandMetTerminationCheckBox;
  SC_IntLineEdit *demandMetTerminationTimeWindow;
  SC_DoubleLineEdit  *demandMetCriteriaRatio;

  // hydraulics
  // Solver GroupBox
  //SC_ComboBox *solver;
  SC_DoubleLineEdit *solverPDARequired;
  SC_ComboBox *solverSelection;
  SC_DoubleLineEdit *solverPDAMin;
  // damage Modeling GroupBox
  SC_TableEdit *pipeDamageModelingTable;
  SC_TableEdit *nodeDamageModelingTable;

  // restoration
  SC_CheckBox *restorationOnCheckBox;
  SC_FileEdit *policyDefinitionFile;
  SC_IntLineEdit *minimumJobTimeLineEdit;
  SC_QRadioButton *pipeLeakBasedRadioButton;
  SC_QRadioButton *pipeTimeBasedRadioButton;
  SC_DoubleLineEdit *pipeDiscoveryLeakAmountLineEdit;
  SC_IntLineEdit *pipeDiscoveryTimeWindowLineEdit;
  SC_TableEdit *pipeTimeBasedDiscoveryTable;
  SC_QRadioButton *nodeLeakBasedRadioButton;
  SC_QRadioButton *nodeTimeBasedRadioButton;
  SC_DoubleLineEdit *nodeDiscoveryLeakAmountLineEdit;
  SC_IntLineEdit *nodeDiscoveryTimeWindowLineEdit;
  SC_TableEdit *nodeTimeBasedDiscoveryTable;
  SC_TableEdit *tankTimeBasedDiscoveryTable;
  SC_TableEdit *pumpTimeBasedDiscoveryTable;

};


#endif // REWET_RECOVERY_H
