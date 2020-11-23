#ifndef CSVtoBIMModelingWidget_H
#define CSVtoBIMModelingWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class QLineEdit;

class CSVtoBIMModelingWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit CSVtoBIMModelingWidget(QWidget *parent);
    virtual ~CSVtoBIMModelingWidget();


    bool outputToJSON(QJsonObject &rvObject);

signals:


private:

    QLineEdit* selectComponentsLineEdit;

};

#endif // CSVtoBIMModelingWidget_H
