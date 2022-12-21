#ifndef LOADRESULTSDIALOG_H
#define LOADRESULTSDIALOG_H

#include <QDialog>

class WorkflowAppWidget;
class QLineEdit;
class WorkflowAppWidget;

class LoadResultsDialog : public QDialog
{
    Q_OBJECT

public:
    LoadResultsDialog(WorkflowAppWidget* parent);

private slots:

    void handleGetPathToInputFile(void);
    void handleGetPathToResultsFolder(void);
    void handleLoadResults(void);
    void handleGetPathToWorkFolder(void);

private:

    QLineEdit* inputFileLineEdit;
    QLineEdit* resultsFolderLineEdit;
    QLineEdit* workFolderLineEdit;

    WorkflowAppWidget* workflowWidget;
};

#endif // LOADRESULTSDIALOG_H
