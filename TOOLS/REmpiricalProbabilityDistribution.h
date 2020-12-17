#ifndef REMPIRICALPROBABILITYDISTRIBUTION_H
#define REMPIRICALPROBABILITYDISTRIBUTION_H
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

#include <math.h>
#include <vector>
#include <QVector>

class REmpiricalProbabilityDistribution
{
public:
    REmpiricalProbabilityDistribution(QString objectName = QString());

    void addSample(const double& val);

    double mean(void);

    double stdDev(void);

    double CV(void);

    QVector<double>  updateHistogram();

    // For plotting
    QVector<double> getRelativeFrequencyDiagram(void);
    QVector<double> getHistogramTicks(void);

    QString getName() const;

    double getHistogramMin() const;

    double getHistogramMax() const;

    double getBinSize() const;

    double getHistPlotHeight() const;

    double getHistogramArea() const;

    int getNumberSamples() const;

    QVector<double> getValues() const;

    double getMax() const;

    double getMin() const;

private:

    QString name;

    QVector<double> values;

    int numBins;
    QVector<double> theFrequencyDiagram;
    double histogramMin;
    double histogramMax;
    double histogramHeight;
    double histPlotHeight;
    double histogramArea;
    double binSize;

    double max;
    double min;
    double parameterSum;
    double parameterSquaredSum;
    int n;
};

#endif // REMPIRICALPROBABILITYDISTRIBUTION_H
