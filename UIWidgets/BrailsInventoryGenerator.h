#ifndef BrailsInventoryGenerator_H
#define BrailsInventoryGenerator_H

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

// Written by: Barbaros Cetiner, Frank McKenna, Stevan Gavrilovic

#include "SimCenterAppWidget.h"
#include "qgsrectangle.h"

class SimCenterMapcanvasWidget;
class QGISVisualizationWidget;
class VisualizationWidget;
class GIS_Selection;
class QLabel;
class SC_DoubleLineEdit;
class QLineEdit;
class QPushButton;
class SC_FileEdit;
class SC_ComboBox;
class BrailsGoogleDialog;

typedef struct regionInputStruct {
    double minLat;
    double maxLat;
    double minLong;
    double maxLong;
    QString location;
    QString outputFile;
} regionData;

typedef struct fpInputStruct {
    double minLat;
    double maxLat;
    double minLong;
    double maxLong;
    QString location;
    QString outputFile;
    QString fpSource;
    QString fpSourceAttrMap;
    QString units;
} fpData;

typedef struct BrailsDataStruct {
  double minLat;
  double maxLat;
  double minLong;
  double maxLong;
  QString locationStr;
  QString fpSource;
  QString fpSourceAttrMap;
  QString outputFile;
  QString imageSource;
  QString imputationAlgo;
  QString units;
  QString location;
} BrailsData;


class BrailsInventoryGenerator : public SimCenterAppWidget
{
    Q_OBJECT

public:
    BrailsInventoryGenerator(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~BrailsInventoryGenerator();

public slots:
    void clear(void);

signals:

protected:

private slots:
  void runBRAILS(void);
  void coordsChanged(void);
  void getLocationBoundary(void);
  void getFootprints(void);
  
private:
  std::unique_ptr<SimCenterMapcanvasWidget> mapViewSubWidget;
  QGISVisualizationWidget* theVisualizationWidget = nullptr;
  
  SC_ComboBox *units;
  SC_ComboBox *location;
  SC_DoubleLineEdit *minLat, *maxLat, *minLong, *maxLong;
  QLabel* regionNameLabel;
  QLineEdit *locationName, *locationStr;
  QLabel* regionGeojsonLabel;
  QLineEdit *regionGeojson;
  QPushButton* browseGeojsonButton;
  SC_ComboBox *footprintSource;
  SC_ComboBox *baselineInvSelection;
  SC_ComboBox *attributeSelected;
  SC_FileEdit *theOutputFile, *fpGeojsonFile, *fpAttrGeojsonFile, *invGeojsonFile, *invAttrGeojsonFile;
  GIS_Selection *theSelectionWidget;

  QgsRectangle zoomRectangle;
  QString printSuffix;
  QString imageSource;
  QString imputationAlgo;

  BrailsGoogleDialog *theGoogleDialog = 0;
};

#endif // BrailsInventoryGenerator_H
