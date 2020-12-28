/*********************************************************************
*                                                                    *
* This file is posted by Dr. Stevan Gavrilovic (steva44@hotmail.com) *
* as a result of work performed in the research group of Professor   *
* Terje Haukaas (terje@civil.ubc.ca) at the University of British    *
* Columbia in Vancouver. The code is part of the computer program    *
* Rts, which is an extension of the computer program Rt developed by *
* Dr. Mojtaba Mahsuli (mahsuli@sharif.edu) in the same group.        *
*                                                                    *
* The content of this file is the product of thesis-related work     *
* performed at the University of British Columbia (UBC). Therefore,  *
* the Rights and Ownership are held by UBC.                          *
*                                                                    *
* Please be cautious when using this code. It is provided “as is”    *
* without warranty of any kind, express or implied.                  *
*                                                                    *
* Contributors to this file:                                         *
*   - Stevan Gavrilovic                                              *
*                                                                    *
*********************************************************************/

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

#include "REmpiricalProbabilityDistribution.h"

#include "QDebug"

REmpiricalProbabilityDistribution::REmpiricalProbabilityDistribution(QString objectName) : name(objectName)
{
    numBins = 60;
    n = 0;
    histogramMin = 0.0;
    histogramMax = 0.0;
    histogramHeight = 0.0;
    histPlotHeight = 0.0;
    histogramArea = 0.0;
    binSize = 0.0;
    parameterSum = 0.0;
    parameterSquaredSum = 0.0;
    max = 0.0;
    min = 0.0;
}


void REmpiricalProbabilityDistribution::addSample(const double& val)
{
    values.push_back(val);
    parameterSum += val;
    parameterSquaredSum += val*val;
    ++n;

    if(val > max)
        max = val;

    if(val < min)
        min = val;
}


double REmpiricalProbabilityDistribution::mean(void)
{
    auto num = static_cast<double>(n);

    return  parameterSum/num;
}


double REmpiricalProbabilityDistribution::stdDev(void)
{

    if(n<=1)
        return 0.0;

    double meanVal = this->mean();

    auto num = static_cast<double>(n);

    return sqrt(1.0/(num-1.0) * (parameterSquaredSum - num * meanVal * meanVal));
}


double  REmpiricalProbabilityDistribution::CV(void)
{
    auto stdv = this->stdDev();
    auto meanVal = this->mean();

    return stdv/meanVal;
}


QVector<double>  REmpiricalProbabilityDistribution::getRelativeFrequencyDiagram(void)
{
    theFrequencyDiagram.clear();

    auto theHistogram = this->updateHistogram();

    auto factor = 1.0/histogramArea;

    // Get sizes
    int vSize = theHistogram.size();

    //resize the frequency diagram
    theFrequencyDiagram.resize(vSize);


    // double area = 0.0;
    for (int i=0; i<vSize; ++i)
    {
        theFrequencyDiagram[i] = factor * theHistogram[i];

        // area += theFrequencyDiagram[i] * binSize;
    }

    // qDebug()<<"area"<<area;

    return theFrequencyDiagram;
}


QVector<double>  REmpiricalProbabilityDistribution::getHistogramTicks(void)
{

    QVector<double> theHistogramTicks;

    auto meanVal = this->mean();

    theHistogramTicks.resize(numBins);

    auto stdv = this->stdDev();

    // Get size of histogram
    histogramMin = meanVal - 5.0 * stdv;
    histogramMax = meanVal + 5.0 * stdv;
    binSize = (histogramMax - histogramMin) / numBins;

    // Set bin ticks
    for (int k=0; k<numBins; ++k)
    {

        theHistogramTicks[k] = histogramMin + k * binSize - 0.5 * binSize;
    }

    return theHistogramTicks;
}


QString REmpiricalProbabilityDistribution::getName() const
{
    return name;
}


double REmpiricalProbabilityDistribution::getHistogramMin() const
{
    return histogramMin;
}


double REmpiricalProbabilityDistribution::getHistogramMax() const
{
    return histogramMax;
}


double REmpiricalProbabilityDistribution::getBinSize() const
{
    return binSize;
}


double REmpiricalProbabilityDistribution::getHistPlotHeight() const
{
    return histPlotHeight;
}


double REmpiricalProbabilityDistribution::getHistogramArea() const
{
    return histogramArea;
}


int REmpiricalProbabilityDistribution::getNumberSamples() const
{
    return n;
}


QVector<double> REmpiricalProbabilityDistribution::getValues() const
{
    return values;
}


double REmpiricalProbabilityDistribution::getMax() const
{
    return max;
}


double REmpiricalProbabilityDistribution::getMin() const
{
    return min;
}


QVector<double>  REmpiricalProbabilityDistribution::updateHistogram()
{
    QVector<double> theHistogram(numBins);

    if(n<1)
    {
        qDebug()<<"Error, need samples to create a histogram";
        return theHistogram;
    }

    auto stdv = this->stdDev();

    auto meanVal = this->mean();

    // Get size of histogram
    histogramMin = meanVal - 5.0 * stdv;
    histogramMax = meanVal + 5.0 * stdv;
    binSize = (histogramMax - histogramMin) / numBins;


    // Create histogram
    for (int j=0; j<values.size(); ++j) {

        for (int k=0; k<numBins; ++k) {

            if (values.at(j) < histogramMin + static_cast<double>(k) * binSize) {

                theHistogram[k] += 1.0;

                if (theHistogram[k] > histogramHeight) {
                    histogramHeight += 1.0;
                }
                break;
            }
        }
    }

    histogramArea = static_cast<double>(n)*binSize;

    if (histogramHeight/histogramArea > histPlotHeight) {

        histPlotHeight = histogramHeight/histogramArea*1.1;
    }

    return theHistogram;
}
