#ifndef TABLEPRINTER_H
#define TABLEPRINTER_H

#include <QTextDocument>

class QTableView;

class TablePrinter
{
public:
    TablePrinter();

    void printToTable(QTextCursor* cursor, QTableView* tableView, const QString& strTitle);

};

#endif // TABLEPRINTER_H
