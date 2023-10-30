#ifndef BRAILS_GOOGLE_DIALOG_H
#define BRAILS_GOOGLE_DIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <BrailsInventoryGenerator.h>

class SC_IntLineEdit;
class QLineEdit;

class BrailsGoogleDialog : public QDialog
{
    Q_OBJECT
public:
  explicit BrailsGoogleDialog(QWidget *parent);
  void setData(BrailsData &);

signals:

public slots:
  void startBrails(void);
  void handleBrailsFinished();
  
private:
  SC_IntLineEdit *numBuildings;
  SC_IntLineEdit *seed;
  BrailsData brailsData;
  QLineEdit *apiKey;
};

#endif // BRAILS_GOOGLE_DIALOG_H
