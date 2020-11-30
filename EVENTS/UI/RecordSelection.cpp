#include "RecordSelection.h"

RecordSelection::RecordSelection(QObject *parent) : QObject(parent)
{
    m_recordId = -1;
    m_scaleFactor = 0.0;
}

int RecordSelection::recordId() const
{
    return m_recordId;
}

void RecordSelection::setRecordId(int recordId)
{
    if(m_recordId != recordId)
    {
        m_recordId = recordId;
        emit recordChanged(m_recordId);
    }
}

double RecordSelection::scaleFactor() const
{
    return m_scaleFactor;
}

void RecordSelection::setScaleFactor(double scaleFactor)
{
    if(m_scaleFactor != scaleFactor)
    {
        m_scaleFactor = scaleFactor;
        emit scaleChanged(m_scaleFactor);
    }
}
