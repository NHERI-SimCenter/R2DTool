#ifndef ComponentInputWidget_H
#define ComponentInputWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

#include <set>

#include <QString>
#include <QObject>

class AssetInputDelegate;

class QGroupBox;
class QLineEdit;
class QTableWidget;
class QLabel;

class ComponentInputWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit ComponentInputWidget(QWidget *parent, QString type);
    virtual ~ComponentInputWidget();

    QGroupBox* getComponentsWidget(void);

    QTableWidget *getTableWidget() const;

    void insertSelectedComponent(const int ComponentID);

    int numberComponentsSelected(void);

    // Set custom labels in widget
    void setComponentType(const QString &value);
    void setLabel1(const QString &value);
    void setLabel2(const QString &value);
    void setLabel3(const QString &value);  
    void setGroupBoxText(const QString &value);

    void testFileLoad(QString& path);

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    QString getPathToComponentFile(void) const;

signals:
    void componentDataLoaded();

public slots:
    void handleComponentSelection(void);

private slots:
    void selectComponents(void);
    void loadComponentData(void);
    void chooseComponentInfoFileDialog(void);
    void clearComponentSelection(void);

private:
    QString pathToComponentInfoFile;
    QLineEdit* componentFileLineEdit;
    AssetInputDelegate* selectComponentsLineEdit;
    QTableWidget* componentTableWidget;
    QLabel* componentInfoText;
    QGroupBox* componentGroupBox;

    QString componentType;
    QString label1;
    QString label2;
    QString label3;

    void createComponentsBox(void);
};

#endif // ComponentInputWidget_H
