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

// Written by:
// Jinyan Zhao

#include "HAZUSDemandWidget.h"
#include "SC_DoubleLineEdit.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QDoubleValidator>

HAZUSDemandWidget::HAZUSDemandWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    layout = new QGridLayout();
    magnitudeLable = new QLabel("Moment Magnitude");
    magnitudeLineEdit = new SC_DoubleLineEdit("EarthquakeMagnitude", //Key
                                              7.0, //initValue
                                              0.0, //min
                                              10.0, //max
                                              2); //decimal
    magnitudeLineEdit->setMaximumWidth(100);
    magnitudeLineEdit->setMinimumWidth(100);

    layout->addWidget(magnitudeLable, 0, 0);
    layout->addWidget(magnitudeLineEdit, 0, 1);

    layout->setRowStretch(1,1);
    layout->setColumnStretch(2,1);

    this->setLayout(layout);

    this->clear();


}


bool HAZUSDemandWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Name"] = "HAZUS";
    QJsonObject paramObj;
    magnitudeLineEdit->outputToJSON(paramObj);
    jsonObject["Parameters"] = paramObj;
    return true;
}


bool HAZUSDemandWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if(jsonObject.contains("EarthquakeMagnitude")){
        double mag = jsonObject["EarthquakeMagnitude"].toDouble(7.0);
        QString magTxt = QString::number(mag);
        magnitudeLineEdit->setText(magTxt);
    }

    return true;
}


void HAZUSDemandWidget::clear(void)
{

}





bool HAZUSDemandWidget::copyFiles(QString &destName)
{

    return true;
}



bool
HAZUSDemandWidget::outputCitation(QJsonObject &jsonObject)
{
  QJsonObject citationPelicun;
  citationPelicun.insert("citation",
"Adam Zsarnoczay, John Vouvakis Manousakis, Jinyan Zhao, Kuanshi Zhong, \
Pouria Kourehpaz (2024). NHERI-SimCenter/pelicun: v3.3. \
Zenodo. https://doi.org/10.5281/zenodo.10896145");
  citationPelicun.insert("description",
"This reference indicates the version of the tool used for the simulation.");

  QJsonObject citationPelicunMarker;
  citationPelicunMarker.insert("citation",
"Adam Zsarnoczay, Gregory G. Deierlein, \
PELICUN - A Computational Framework for Estimating Damage, Loss, and Community \
Resilience, \
Proceedings of the 17th World Conference on Earthquake Engineering, Japan, 2020");
  citationPelicunMarker.insert("description",
"This paper describes the Pelicun damage and loss assessment framework. Please \
reference it if your work results from using the Pelicun engine in the SimCenter \
tools.");

  QJsonArray citationsArray;
  citationsArray.push_back(citationPelicun);
  citationsArray.push_back(citationPelicunMarker);

  jsonObject.insert("citations", citationsArray);
  
  return true;
}

