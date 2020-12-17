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

    auto theHistogram = this->updateHistogram();

    auto factor = 1.0/histogramArea;

    // Get sizes
    int vSize = theHistogram.size();
    theFrequencyDiagram.clear();

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
    QVector<double> theHistogramTicks(numBins);

    auto stdv = this->stdDev();

    auto meanVal = this->mean();

    // Get size of histogram
    histogramMin = meanVal - 5.0 * stdv;
    histogramMax = meanVal + 5.0 * stdv;
    binSize = (histogramMax - histogramMin) / numBins;

    // Set bin ticks
    for (int k=0; k<numBins; ++k) {

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

    if(n<=1)
    {
        qDebug()<<"Error, need more than 1 sample to create a histogram";
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
