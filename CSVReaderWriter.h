#ifndef CSVREADERWRITER_H
#define CSVREADERWRITER_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include <QVector>

class QString;
class QStringList;

class CSVReaderWriter
{
public:
    CSVReaderWriter();

    // Saves data in the format of a CSV file

    int saveCSVFile(const QVector<QStringList>& data, const QString& pathToFile, QString& err);

    // Parses a CSV file and returns the file as a vector of string lists
    // Each item in the vector (string list) corresponds to a row of the csv file that is parsed
    // The string list corresponds to the items within a row, i.e., the values in the cells. There are as many items in the string list as there are in the row of the CSV file
    QVector<QStringList> parseCSVFile(const QString &pathToFile, QString& err);

private:

    QStringList parseLineCSV(const QString &csvString);

};

#endif // CSVREADERWRITER_H
