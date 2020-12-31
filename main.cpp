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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "AgaveCurl.h"
#include "GoogleAnalytics.h"
#include "MainWindowWorkflowApp.h"
#include "RDTUserPass.h"
#include "WorkflowAppRDT.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QOpenGLWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextStream>
#include <QThread>
#include <QTime>

#include "ArcGISRuntimeEnvironment.h"

using namespace Esri::ArcGISRuntime;

static QString logFilePath;
static bool logToFile = false;

// customMessgaeOutput code taken from web:
// https://stackoverflow.com/questions/4954140/how-to-redirect-qdebug-qwarning-qcritical-etc-output

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile) {
        QString txt = QString("%1 %2: %3 (%4)").arg(formattedTime, logLevelName, msg,  context.file);
        QFile outFile(logFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << "\n";
        outFile.close();
    } else {
        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}


int main(int argc, char *argv[])
{

#ifdef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Setting Core Application Name, Organization, Version and Google Analytics Tracking Id
    QCoreApplication::setApplicationName("RDT");
    QCoreApplication::setOrganizationName("SimCenter");
    QCoreApplication::setApplicationVersion("1.0.0");

    // GoogleAnalytics::SetTrackingId("UA-186298856-1");
    // GoogleAnalytics::StartSession();
    // GoogleAnalytics::ReportStart();

    // set up logging of output messages for user debugging
    logFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QDir::separator() + QCoreApplication::applicationName();

    // make sure tool dir exists in Documentss folder
    QDir dirWork(logFilePath);
    if (!dirWork.exists())
        if (!dirWork.mkpath(logFilePath))   {
            qDebug() << QString("Could not create Working Dir: ") << logFilePath;
        }

    // Full path to debug.log file
    logFilePath = logFilePath + QDir::separator() + QString("debug.log");

    // Remove old log file
    QFile debugFile(logFilePath);
    debugFile.remove();

    QByteArray envVar = qgetenv("QTDIR");  //  check if the app is run in Qt Creator

    if (envVar.isEmpty())
        logToFile = true;

    qInstallMessageHandler(customMessageOutput);

    // window scaling
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    /******************code to reset openGL version .. keep around in case need again
    QSurfaceFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glFormat);
    *********************************************************************************/

    // Regular Qt startup
    QApplication a(argc, argv);

    // Set the key for the ArcGIS interface
    ArcGISRuntimeEnvironment::setLicense(getArcGISKey());

    // create a remote interface
    QString tenant("designsafe");
    QString storage("agave://designsafe.storage.default/");
    QString dirName("RDT");

    AgaveCurl *theRemoteService = new AgaveCurl(tenant, storage, &dirName);


    // Create the main window
    WorkflowAppRDT *theInputApp = new WorkflowAppRDT(theRemoteService);
    MainWindowWorkflowApp w(QString("RDT: Regional Resilience Determination Tool"), theInputApp, theRemoteService);

    // Create the  menu bar and actions to run the examples
    theInputApp->initialize();


    QString aboutTitle = "About the SimCenter RDT Application"; // this is the title displayed in the on About dialog
    QString aboutSource = ":/resources/docs/textAboutRDT.html";  // this is an HTML file stored under resources

    w.setAbout(aboutTitle, aboutSource);

    QString version("Version 1.0.0");
    w.setVersion(version);

    QString citeText("TO DO RDT CITATION");
    w.setCite(citeText);

    QString manualURL("https://nheri-simcenter.github.io/RDT-Documentation/");
    w.setDocumentationURL(manualURL);

    QString messageBoardURL("https://simcenter-messageboard.designsafe-ci.org/smf/index.php?board=6.0");
    w.setFeedbackURL(messageBoardURL);


    // Move remote interface to a thread
    QThread *thread = new QThread();
    theRemoteService->moveToThread(thread);

    QWidget::connect(thread, SIGNAL(finished()), theRemoteService, SLOT(deleteLater()));
    QWidget::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();

    // Show the main window, set styles & start the event loop
    w.show();
    w.statusBar()->showMessage("Ready", 5000);

#ifdef Q_OS_WIN
    QFile commonStyleSheetFile(":/styleCommon/stylesheetWIN.qss");
#endif

#ifdef Q_OS_MACOS
    QFile commonStyleSheetFile(":/styleCommon/stylesheetMAC.qss");
#endif

#ifdef Q_OS_LINUX
    QFile commonStyleSheetFile(":/styleCommon/stylesheetMAC.qss");
#endif

    QFile RDTstyleSheetFile(":/styles/stylesheetRDT.qss");

    if(commonStyleSheetFile.open(QFile::ReadOnly) && RDTstyleSheetFile.open(QFile::ReadOnly))
    {
        auto commonStyleSheet = commonStyleSheetFile.readAll();
        auto RDTStyleSheet = RDTstyleSheetFile.readAll();

        // Append the RDT stylesheet to the common stylesheet
        commonStyleSheet.append(RDTStyleSheet);

        a.setStyleSheet(commonStyleSheet);
        commonStyleSheetFile.close();
        RDTstyleSheetFile.close();
    }
    else
    {
        qDebug() << "could not open stylesheet";
    }


    int res = a.exec();

    // On done with event loop, logout & stop the thread
    theRemoteService->logout();
    thread->quit();

    // GoogleAnalytics::EndSession();

    return res;
}
