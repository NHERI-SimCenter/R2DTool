#ifndef PelicunDLWidget_H
#define PelicunDLWidget_H

#include "SimCenterAppWidget.h"

class QComboBox;
class QLineEdit;
class QCheckBox;

class PelicunDLWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit PelicunDLWidget(QWidget *parent = nullptr);

    bool outputAppDataToJSON(QJsonObject &jsonObject);

signals:

public slots:

private:

    QComboBox* DLTypeComboBox;
    QLineEdit* realizationsLineEdit;
    QComboBox* eventTimeComboBox;
    QCheckBox* detailedResultsCheckBox;
    QCheckBox* logFileCheckBox;
    QCheckBox* coupledEDPCheckBox;
    QCheckBox* groundFailureCheckBox;

};

#endif // PelicunDLWidget_H
