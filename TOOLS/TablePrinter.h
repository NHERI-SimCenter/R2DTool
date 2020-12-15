#ifndef TABLEPRINTER_H
#define TABLEPRINTER_H

#include <QTextDocument>

class QTableView;

class TablePrinter
{
public:
    TablePrinter();

    QTextDocument* printToTable(QTableView* tableView, const QString& strTitle);

};

#endif // TABLEPRINTER_H
