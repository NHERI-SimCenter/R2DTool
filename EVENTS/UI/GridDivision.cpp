#include "GridDivision.h"

GridDivision::GridDivision(QObject *parent) : QObject(parent)
{

}

double GridDivision::getStep()
{
    return (m_max - m_min)/m_divisions;
}

double GridDivision::min()
{
    return m_min;
}

void GridDivision::setMin(double min)
{
    if(m_min != min)
    {
        m_min = min;
        emit minChanged(m_min);
    }
}

double GridDivision::max()
{
    return m_max;
}

void GridDivision::setMax(double max)
{
    if(m_max != max)
    {
        m_max = max;
        emit maxChanged(m_max);
    }
}

void GridDivision::set(double min, double max, int divisions)
{
    setMin(min);
    setMax(max);
    setDivisions(divisions);
}

int GridDivision::divisions()
{
    return m_divisions;
}

void GridDivision::setDivisions(int divisions)
{
    if(m_divisions != divisions)
    {
        m_divisions = divisions;
        emit divisionsChanged(m_divisions);
    }
}
