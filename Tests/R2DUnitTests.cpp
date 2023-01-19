/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
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

// Created by: Dr. Stevan Gavrilovic, UC Berkeley

#include "AgaveCurl.h"
#include "WorkflowAppR2D.h"
#include "MainWindowWorkflowApp.h"
#include "LocalApplication.h"
#include "SimCenterPreferences.h"

#include <QRegExp>
#include <QCoreApplication>
#include <QtTest/QtTest>

class R2DUnitTests: public QObject
{

    Q_OBJECT

public:
    R2DUnitTests()
    {
        // create a remote interface
        QString tenant("designsafe");
        QString storage("agave://designsafe.storage.default/");
        QString dirName("R2D");

        theRemoteService = new AgaveCurl(tenant, storage, &dirName);

        theInputApp = new WorkflowAppR2D(theRemoteService);

        mainWindow = new MainWindowWorkflowApp(QString("R2D: Regional Resilience Determination Tool"), theInputApp, theRemoteService);

        // Create the  menu bar and actions to run the examples
        theInputApp->initialize();

        // Timeout to run each example
        // Set 45 minute test timeout
        qputenv("QTEST_FUNCTION_TIMEOUT", QByteArray("2700000"));

        QCoreApplication::setApplicationName("R2D");

    }

private slots:
    void testExamples();

private:

    AgaveCurl *theRemoteService = nullptr;
    WorkflowAppR2D *theInputApp = nullptr;
    MainWindowWorkflowApp* mainWindow = nullptr;
};


void R2DUnitTests::testExamples()
{

    auto result = qgetenv("QTEST_FUNCTION_TIMEOUT");
    qDebug() << "Timeout for total test set to:" << result.toInt()/1000/60 << " min";

    //    QString str = "Hello";
    //    QCOMPARE(str.toUpper(), QString("HELLO"));

    auto pathToExamples = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples" + QDir::separator();

    QVERIFY2(QFileInfo::exists(pathToExamples), "No examples directory found");

    // Iterate through the directories and find all of the example directories
    QStringList exList;
    QDirIterator dirt(pathToExamples, QDir::Dirs);
    while (dirt.hasNext())
    {
        dirt.next();
        exList.append(dirt.filePath());
    }

    QVERIFY2(!exList.empty(), "No examples found in examples directory");

    // Iterate through each example and run it
    for(auto&& it : exList)
    {

        // Check if the input file exists
        QString pathInputFile = it + QDir::separator() + "input.json";

        if(!QFileInfo::exists(pathInputFile))
            continue;

        qDebug()<<"***** Start running example "<<it<<" *****"<<Qt::endl;

        qDebug()<<pathInputFile<<Qt::endl;

        auto res = mainWindow->loadFile(pathInputFile);

        QVERIFY2(res == 0, "Failed loading the example "+pathInputFile.toLocal8Bit());

        LocalApplication* localApp = theInputApp->getLocalApp();

        // Event loop to wait until example is done running
        QEventLoop loop;

        connect( localApp, &LocalApplication::runComplete, &loop, &QEventLoop::quit );
        mainWindow->onRunButtonClicked();

        loop.exec();

        auto pathToWorkDir = SimCenterPreferences::getInstance()->getLocalWorkDir() + QDir::separator() + "tmp.SimCenter";

        qDebug()<<"Path to work dir: "<<pathToWorkDir<<Qt::endl;

        auto pathToResultsDir = pathToWorkDir + QDir::separator() + "Results";

        qDebug()<<"Path to results dir: "<<pathToResultsDir<<Qt::endl;

        // Check that the 'Results' folder exists
        QVERIFY2(QFileInfo::exists(pathToResultsDir), "Results directory not found: " + pathToResultsDir.toLocal8Bit());

        QStringList filesList;
        QDirIterator dirResIt(pathToResultsDir, QDir::Files);
        while (dirResIt.hasNext())
        {
            dirResIt.next();
            filesList.append(dirResIt.fileName());

            // qDebug()<<dirResIt.fileName();
        }

        // Get the decision variable file containing the final results
        QRegExp rxDV("(?:DV_)\\d*[-]\\d*(?:\\.csv)");
        auto DVfileName = filesList.filter(rxDV);

        QVERIFY2(DVfileName.size() == 1, "Could not find the final DV file with results. Should be in the format: DV_*-*.csv");

        auto pathToDVFile = pathToResultsDir + QDir::separator() + DVfileName.first();

        qDebug()<<"Path to DV file: "<<pathToDVFile<<Qt::endl;


        QFile DVFile(pathToDVFile);

        QVERIFY2(DVFile.open(QIODevice::ReadOnly),"Cannot open the file: " + pathToDVFile.toLocal8Bit());

        // Extract the data from the DV file
        QStringList rowLines;
        while (!DVFile.atEnd())
        {
            QString line = DVFile.readLine();

            rowLines << line;
        }

        auto numRows = rowLines.size();

        QVERIFY2(numRows != 0,"The DV file is empty");

        qDebug()<<"***** Finished running example "<<it<<" *****"<<Qt::endl;

    }

}



QTEST_MAIN(R2DUnitTests)
#include "R2DUnitTests.moc"
