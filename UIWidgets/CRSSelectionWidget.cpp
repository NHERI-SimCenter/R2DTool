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

// Written by: Stevan Gavrilovic

#include "CRSSelectionWidget.h"

#include <QHBoxLayout>
#include <QLabel>

#include <qgsprojectionselectionwidget.h>
#include <qgsproject.h>

CRSSelectionWidget::CRSSelectionWidget(QWidget* parent) : QWidget(parent)
{

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    mCrsSelector = new QgsProjectionSelectionWidget();
    mCrsSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    mCrsSelector->setObjectName(QString::fromUtf8("mCrsSelector"));
    mCrsSelector->setFocusPolicy(Qt::StrongFocus);

    connect(mCrsSelector,&QgsProjectionSelectionWidget::crsChanged,this,[=](const QgsCoordinateReferenceSystem& val){emit crsChanged(val);});

    QLabel* crsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    mainLayout->addWidget(crsTypeLabel);
    mainLayout->addWidget(mCrsSelector);
}



bool CRSSelectionWidget::inputAppDataFromJSON(const QJsonObject &jsonObj, QString& errMsg)
{
    // Set the CRS
#ifdef OpenSRA
    // hard code CRS to 4326 until Steve updates this
    auto crsValue = QString::fromUtf8("EPSG:4326");
    //
#else
    auto crsValue = jsonObj["CRS"].toString();
#endif

    if(crsValue.isEmpty())
    {
        errMsg = "Warning: No coordinate reference system provided for raster layer, using project CRS. Check and change if necessary.";
        QgsProject::instance()->crs();

        auto projectCrs = QgsProject::instance()->crs();
        mCrsSelector->setCrs(projectCrs);

        return false;
    }
    else
    {
        QgsCoordinateReferenceSystem newCrs(crsValue);

        if(!newCrs.isValid())
        {
            errMsg = "Warning: the provided coordinate reference system "+crsValue+" is not valid, using project crs. Check and change if necessary.";
            auto projectCrs = QgsProject::instance()->crs();
            mCrsSelector->setCrs(projectCrs);

            return false;
        }
        else
        {
            mCrsSelector->setCrs(newCrs);
        }
    }

    return true;
}


void CRSSelectionWidget::setCRS(const QgsCoordinateReferenceSystem & val)
{
    mCrsSelector->setCrs(val);
}


void CRSSelectionWidget::clear(void)
{
    mCrsSelector->setCrs(QgsCoordinateReferenceSystem());
}


bool CRSSelectionWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["CRS"] = mCrsSelector->crs().authid();

    return true;
}


