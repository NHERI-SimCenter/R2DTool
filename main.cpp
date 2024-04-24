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
#include "WorkflowAppR2D.h"

#ifdef INCLUDE_USER_PASS
#include "R2DUserPass.h"
#else
#include "SampleUserPass.h"
#endif

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
#include <QWebEngineView>

#include "qgsapplication.h"

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
    {
        qDebug()<<msg;
        abort();
    }
}


int main(int argc, char *argv[])
{

#ifdef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute( Qt::AA_UseHighDpiPixmaps );

    // Setting Core Application Name, Organization, Version
    QCoreApplication::setApplicationName("R2D");
    QCoreApplication::setOrganizationName("SimCenter");
    QCoreApplication::setApplicationVersion("4.1.0");

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

    /******************code to reset openGL version .. keep around in case need again
    QSurfaceFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glFormat);
    *********************************************************************************/


    // Start the Application
    QgsApplication a( argc, argv, true );

    //Setting Google Analytics Tracking Information
    GoogleAnalytics::SetMeasurementId("G-ZXJJP9JW1R");
    GoogleAnalytics::SetAPISecret("UPiFP4sETYedbPqIhVdCDA");
    GoogleAnalytics::CreateSessionId();
    GoogleAnalytics::StartSession();


    // create a remote interface
    QString tenant("designsafe");
    QString storage("agave://designsafe.storage.default/");
    QString dirName("R2D");

    AgaveCurl *theRemoteService = new AgaveCurl(tenant, storage, &dirName);


    // Create the main window
    WorkflowAppR2D *theInputApp = new WorkflowAppR2D(theRemoteService);
    MainWindowWorkflowApp w(QString("R2D: Regional Resilience Determination Tool"), theInputApp, theRemoteService, 0, true);

    // Create the  menu bar and actions to run the examples
    theInputApp->initialize();

    QString aboutTitle = "About the SimCenter R2D Application"; // this is the title displayed in the on About dialog
    QString aboutSource = ":/resources/docs/textAboutR2D.html";  // this is an HTML file stored under resources

    w.setAbout(aboutTitle, aboutSource);

    QString version = QString("Version ") + QCoreApplication::applicationVersion();

    w.setVersion(version);

    QString citeText("\n1)Frank McKenna, Stevan Gavrilovic, Zsarnoczay, A., Zhao, J., Zhong, K., Barbaros Cetiner, Yi, S.-. ri ., Elhaddad, W., & Arduino, P. (2024). NHERI-SimCenter/R2DTool: Version 4.0.0 (v4.0.0). Zenodo. https://doi.org/10.5281/zenodo.10448043 \n\n2) Gregory G. Deierlein, Frank McKenna, Adam Zsarnóczay, Tracy Kijewski-Correa, Ahsan Kareem, Wael Elhaddad, Laura Lowes, Matt J. Schoettler, and Sanjay Govindjee (2020) A Cloud-Enabled Application Framework for Simulating Regional-Scale Impacts of Natural Hazards on the Built Environment. Frontiers in the Built Environment. 6:558706. doi: 10.3389/fbuil.2020.558706");
    w.setCite(citeText);

    QString manualURL("https://nheri-simcenter.github.io/R2D-Documentation/");
    w.setDocumentationURL(manualURL);

    QString messageBoardURL("http://simcenter-messageboard.designsafe-ci.org/smf/index.php?board=8.0");
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

    QFile R2DstyleSheetFile(":/styles/stylesheetR2D.qss");

    if(commonStyleSheetFile.open(QFile::ReadOnly) && R2DstyleSheetFile.open(QFile::ReadOnly))
    {
        auto commonStyleSheet = commonStyleSheetFile.readAll();
        auto R2DStyleSheet = R2DstyleSheetFile.readAll();

        // Append the stylesheet to the common stylesheet
        commonStyleSheet.append(R2DStyleSheet);

        a.setStyleSheet(commonStyleSheet);
        commonStyleSheetFile.close();
        R2DstyleSheetFile.close();
    }
    else
    {
        qDebug() << "could not open stylesheet";
    }

    //
    //Setting Google Analytics Tracking Information
    //
    
    /* *******************************************************************

    GoogleAnalytics::SetMeasurementId("G-ZXJJP9JW1R");
    GoogleAnalytics::SetAPISecret("UPiFP4sETYedbPqIhVdCDA");
    GoogleAnalytics::CreateSessionId();
    GoogleAnalytics::StartSession();

    // Opening a QWebEngineView and using github to get app geographic usage
    QWebEngineView view;
    view.setUrl(QUrl("https://nheri-simcenter.github.io/R2DTool/GA4.html"));
    view.resize(1024, 750);
    view.show();
    view.hide();
    ******************************************************************** */
    

    int res = a.exec();

    // On done with event loop, logout & stop the thread
    theRemoteService->logout();
    thread->quit();

    GoogleAnalytics::EndSession();

    return res;
}
