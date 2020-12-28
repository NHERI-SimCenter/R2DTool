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

#include "PointSourceRupture.h"

PointSourceRupture::PointSourceRupture(double magnitude, const RuptureLocation &location, double averageRake, double averageDip, QObject *parent): QObject(parent), m_location(location.latitude(), location.longitude())
{
    this->setMagnitude(magnitude);
    this->setAverageDip(averageDip);
    this->setAverageRake(averageRake);
}


double PointSourceRupture::magnitude() const
{
    return m_magnitude;
}


void PointSourceRupture::setMagnitude(double magnitude)
{
    if(this->m_magnitude != magnitude)
    {
        m_magnitude = magnitude;
        emit this->magnitudeChanged(this->m_magnitude);
    }
}


double PointSourceRupture::averageRake() const
{
    return m_averageRake;
}


void PointSourceRupture::setAverageRake(double averageRake)
{
    if(this->m_averageRake != averageRake)
    {
        m_averageRake = averageRake;
        emit this->rakeChanged(this->m_averageRake);
    }
}


QJsonObject PointSourceRupture::getJson()
{
    QJsonObject rupture;
    rupture.insert("Type", "PointSource");
    rupture.insert("Magnitude", m_magnitude);
    QJsonObject location;
    location.insert("Latitude", m_location.latitude());
    location.insert("Longitude", m_location.longitude());
    location.insert("Depth", m_location.depth());
    rupture.insert("Location", location);
    rupture.insert("AverageRake", m_averageRake);
    rupture.insert("AverageDip", m_averageDip);

    return rupture;
}


double PointSourceRupture::averageDip() const
{
    return m_averageDip;
}


void PointSourceRupture::setAverageDip(double averageDip)
{
    if(this->m_averageDip != averageDip)
    {
        m_averageDip = averageDip;
        emit this->dipChanged(this->m_averageDip);
    }
}


RuptureLocation &PointSourceRupture::location()
{
    return this->m_location;
}
