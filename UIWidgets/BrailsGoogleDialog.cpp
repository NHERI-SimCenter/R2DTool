#include "BrailsGoogleDialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>
#include <QCheckBox>

#include "ModularPython.h"
#include "SimCenterPreferences.h"
#include "SC_IntLineEdit.h"

BrailsGoogleDialog::BrailsGoogleDialog(QWidget *parent)
  : QDialog(parent)
{
    this->setWindowTitle("Using Google API");
    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Google API Key"), 0, 0, 1, 2);
    auto apiKeyLabel =
      new QLabel("If you dont have a Google API key, you can <a href=\"https://developers.google.com/maps/documentation/embed/get-api-key#:~:text=Go%20to%20the%20Googlele%20Maps%20Platform%20%3E%20Credentials%20page.&text=On%20the%20Credentials%20page%2C%20click,your%20newly%20created%20API%20key\">follow the instructions</a>");
    apiKeyLabel->setTextFormat(Qt::RichText);
    apiKeyLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    apiKeyLabel->setOpenExternalLinks(true);
    layout->addWidget(apiKeyLabel, 1, 0, 1, 2);
    
    layout->addWidget(new QLabel("API Key"), 2, 0);

    // fill in key if already saved
    apiKey = new QLineEdit();
    apiKey->setEchoMode(QLineEdit::Password);
    layout->addWidget(apiKey, 2, 1);

    QSettings settings;
    auto googleAPISetting = settings.value("GoogleAPI");
    if(googleAPISetting.isValid() && !googleAPISetting.isNull())
        apiKey->setText(googleAPISetting.toString());
   
    //Save Password
    auto savePassword = new QPushButton("Save Key");
    layout->addWidget(savePassword,2,2);
    connect(savePassword,&QPushButton::clicked, this, [=]() {
      QSettings settings;
      settings.setValue("GoogleAPI", apiKey->text());
    });
    
    layout->addWidget(new QLabel("# Images"),3,0);
    numBuildings = new SC_IntLineEdit("numBuildings",10);
    layout->addWidget(numBuildings, 3,1);

    layout->addWidget(new QLabel("Seed"),4,0);
    seed = new SC_IntLineEdit("seed",10);
    layout->addWidget(seed, 4,1);

    QPushButton *runButton = new QPushButton(tr("Get Images and Process"));
    layout->addWidget(runButton, 5,0,1,3);
    connect(runButton,SIGNAL(clicked()),this,SLOT(startBrails()));
}

void
BrailsGoogleDialog::setData(BrailsData &theData){
  brailsData = theData;
}


void
BrailsGoogleDialog::startBrails(void){

  QString appDir = SimCenterPreferences::getInstance()->getAppDir();
  QDir scriptDir(appDir + QDir::separator());
  scriptDir.cd("applications");
  scriptDir.cd("tools");
  scriptDir.cd("BRAILS");
  QString brailsScript = scriptDir.absoluteFilePath("runBrails.py");  

  
  QStringList scriptArgs;
  scriptArgs << QString("--latMin")  << QString::number(brailsData.minLat)
	     << QString("--latMax")  << QString::number(brailsData.maxLat)
	     << QString("--longMin") << QString::number(brailsData.minLong)
	     << QString("--longMax") << QString::number(brailsData.maxLong)
	     << QString("--lengthUnit") << brailsData.units 
	     << QString("--seed")    << QString::number(seed->getInt())
	     << "--numBuildings" << QString::number(numBuildings->getInt())
	     << "--googKey" << apiKey->text()
	     << "--outputFile" << brailsData.outputFile;
			      
  QFileInfo fileInfo(brailsData.outputFile);
  QString outputPath = fileInfo.absolutePath();
  QString fileName = fileInfo.baseName();
  qDebug() << "BRAILS script: " << brailsScript;
  qDebug() << "BRAILS args: " << scriptArgs;
  ModularPython *thePy = new ModularPython(outputPath);
  thePy->run(brailsScript,scriptArgs);

  this->hide();
}

void
BrailsGoogleDialog::handleBrailsFinished(){
  qDebug() << "Brails Finished!\n";
}
