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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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
// Written: fmk

#include "PyrecodesUI.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTabWidget>

#include <SC_FileEdit.h>
#include <SimCenterWidget.h>
#include <PyrecodesSystemConfig.h>
#include <PyrecodesComponentLibrary.h>

// A light, self-contained stylesheet for the pyrecodes UI. Scoped to this
// widget subtree (set on PyrecodesUI) so it tidies up the look without changing
// the host application's global style.
static const char *kPyrecodesStyle = R"(
QGroupBox {
    font-weight: 600;
    border: 1px solid #c8ccd1;
    border-radius: 6px;
    margin-top: 14px;
    padding: 8px 8px 6px 8px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: #2c3e50;
}
QTabWidget::pane {
    border: 1px solid #c8ccd1;
    border-radius: 6px;
    top: -1px;
}
QTabBar::tab {
    padding: 7px 18px;
    margin-right: 2px;
    border: 1px solid #c8ccd1;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    background: #eceff2;
}
QTabBar::tab:selected {
    background: #ffffff;
    color: #1a5276;
    font-weight: 600;
}
/* the two top-level tabs (Component Library / System Configuration) - wider */
QTabWidget#pyrecodesMainTabs > QTabBar::tab {
    min-width: 240px;
    padding: 10px 36px;
    font-size: 14px;
}
QPushButton {
    padding: 5px 14px;
    border: 1px solid #b6bcc2;
    border-radius: 5px;
    background: #f5f6f8;
}
QPushButton:hover  { background: #e8f0fe; border-color: #5b9bd5; }
QPushButton:pressed { background: #d6e4f7; }
QHeaderView::section {
    background: #eef2f7;
    padding: 5px;
    border: none;
    border-right: 1px solid #d7dbe0;
    border-bottom: 1px solid #c8ccd1;
    font-weight: 600;
    color: #2c3e50;
}
QTableWidget {
    gridline-color: #e4e7ea;
    alternate-background-color: #f6f8fa;
    selection-background-color: #d6e4f7;
    selection-color: #000000;
}
QLineEdit, QComboBox, QPlainTextEdit {
    border: 1px solid #c3c8cd;
    border-radius: 4px;
    padding: 3px 5px;
    background: #ffffff;
}
QLineEdit:focus, QComboBox:focus, QPlainTextEdit:focus { border-color: #5b9bd5; }
)";

PyrecodesUI::PyrecodesUI(QWidget *parent)
   : SimCenterAppWidget(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this); // Set the parent to 'this'
  this->setLayout(mainLayout);

  theComponentLibrary = new PyrecodesComponentLibrary();
  theSystemConfiguration = new PyrecodesSystemConfig();

  QTabWidget *theTabWidget = new QTabWidget();
  theTabWidget->setObjectName("pyrecodesMainTabs");
  theTabWidget->addTab(theComponentLibrary, "Component Library");
  theTabWidget->addTab(theSystemConfiguration, "System Configuration");

  //
  // "Generate Main File" row: writes the small pyrecodes Main file that ties
  // the saved Component Library and System Configuration together into a
  // runnable input set.
  //

  QLabel *mainHint = new QLabel("Step 3 – once both files above are saved, generate the pyrecodes Main file that links them:");
  mainHint->setWordWrap(true);
  mainHint->setStyleSheet("color: #555; font-style: italic;");

  QPushButton *generateMainButton = new QPushButton("Generate Main File…");
  generateMainButton->setToolTip("Write the pyrecodes Main JSON that references the saved Component\n"
				 "Library and System Configuration files (the third input pyrecodes needs).");

  connect(generateMainButton, &QPushButton::clicked, this, [=]() {

    QString clPath = theComponentLibrary->getFileName();
    QString scPath = theSystemConfiguration->getFileName();

    if (clPath.isEmpty() || clPath == "<template>" ||
	scPath.isEmpty() || scPath == "<template>") {
      QMessageBox::warning(this, "Generate Main File",
	"Please Save the Component Library and the System Configuration to files first.\n\n"
	"The Main file references them by path, so they must exist on disk.");
      return;
    }

    QString mainName = QFileDialog::getSaveFileName(this, "Save Main File", "Main.json",
						    "JSON files (*.json);;All files (*)");
    if (mainName.isEmpty())
      return;

    QDir mainDir = QFileInfo(mainName).dir();

    QJsonObject cl;
    cl["ComponentLibraryCreatorFileName"]  = "json_component_library_creator";
    cl["ComponentLibraryCreatorClassName"] = "JSONComponentLibraryCreator";
    cl["ComponentLibraryFile"]             = mainDir.relativeFilePath(clPath);

    QJsonObject sys;
    sys["SystemCreatorClassName"]   = "ConcreteSystemCreator";
    sys["SystemCreatorFileName"]    = "concrete_system_creator";
    sys["SystemClassName"]          = "BuiltEnvironment";
    sys["SystemFileName"]           = "built_environment";
    sys["SystemConfigurationFile"]  = mainDir.relativeFilePath(scPath);

    QJsonObject mainObj;
    mainObj["ComponentLibrary"] = cl;
    mainObj["System"]           = sys;

    QFile file(mainName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::warning(this, "Generate Main File", "Could not write file:\n" + mainName);
      return;
    }
    file.write(QJsonDocument(mainObj).toJson());
    file.close();

    QMessageBox::information(this, "Generate Main File",
      "Main file written:\n" + mainName + "\n\n"
      "Run the simulation with:\n"
      "    from pyrecodes import main\n"
      "    main.run('" + mainDir.absolutePath() + "/', '" + QFileInfo(mainName).fileName() + "')");
  });

  // label for the citation
  QLabel *citation = new QLabel("Users should cite this as follows: "
				"Blagojević, Nikola, and Stojadinović, Božidar. (2023). "
				"pyrecodes: an open-source library for regional recovery simulation and disaster resilience assessment of the built environment (v0.1.0). Chair of Structural Dynamics and Earthquake Engineering, ETH Zurich. https://doi.org/10.5905/ethz-1007-700");

  citation->setWordWrap(true);

  mainLayout->addWidget(theTabWidget,        0, 0, 1, 4);
  mainLayout->addWidget(mainHint,            1, 0, 1, 3);
  mainLayout->addWidget(generateMainButton,  1, 3, 1, 1);
  mainLayout->addWidget(citation,            2, 0, 1, 4);
  mainLayout->setRowStretch(0, 1);

  this->setStyleSheet(kPyrecodesStyle);
}


PyrecodesUI::~PyrecodesUI()
{

}


void PyrecodesUI::clear(void)
{

}


bool PyrecodesUI::inputFromJSON(QJsonObject &jsonObject)
{
  Q_UNUSED(jsonObject);
  return true;  
}


bool PyrecodesUI::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "PyrecodesUI";

  QJsonObject componentLibraryData;
  theComponentLibrary->outputToJSON(componentLibraryData);
  jsonObject["componentLibrary"] = componentLibraryData;

  QJsonObject systemData;  
  theSystemConfiguration->outputToJSON(systemData);
  jsonObject["systemConfiguration"] = systemData;
  
  return true;
}

bool PyrecodesUI::outputAppDataToJSON(QJsonObject &jsonObject) {

  jsonObject["Application"] = "PyrecodesUI";
  QJsonObject dataObj;
  jsonObject["ApplicationData"] = dataObj;
  
  return true;
}

bool PyrecodesUI::inputAppDataFromJSON(QJsonObject &jsonObject) {
  Q_UNUSED(jsonObject);    
  return true;
}


bool PyrecodesUI::copyFiles(QString &destDir) {

  return true;
}

bool PyrecodesUI::outputCitation(QJsonObject &citation){
  citation.insert("PyReCoDes",QString("Blagojević, Nikola, and Stojadinović, Božidar. (2023). pyrecodes: an open-source library for regional recovery simulation and disaster resilience assessment of the built environment (v0.1.0). Chair of Structural Dynamics and Earthquake Engineering, ETH Zurich. https://doi.org/10.5905/ethz-1007-700"));
  
  return true;
}

SC_ResultsWidget* PyrecodesUI::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{

  qDebug() << "ERROR: SC_ResultsWidget* PyrecodesUI::getResultsWidget()";
  return 0;
  
}
