#include "CSVReaderWriter.h"

#include <QVector>
#include <QTextStream>
#include <QStringList>
#include <QFile>

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

CSVReaderWriter::CSVReaderWriter()
{

}


int CSVReaderWriter::saveCSVFile(const QVector<QStringList>& data, const QString& pathToFile, QString& err)
{

    // Check the data for consistency
    if(data.empty())
    {
        err = "Empty data vector came into the function save data.";
        return -1;
    }

    auto numCol = data.first().size();

    // Check that there are items in each row and that the number of items is consistent
    if(numCol==0)
    {
        err = "Empty data vector came into the function save data.";
        return -1;
    }

    for(auto&& row : data)
    {
        if(row.size() != numCol)
        {
            err = "Inconsistency between the column sizes in the data.";
            return -1;
        }
    }

    QFile file(pathToFile);

    if (!file.open(QIODevice::WriteOnly))
    {
        err = "Cannot create the file: " + pathToFile + "\n" +"Check your directory and try again.";
        return -1;
    }

    QTextStream csvFileOut(&file);

    // Function to handle the case where there are commas within a cell
    auto strOutput = [](const QString& in)
    {
        if(in.contains(','))
            return "\"" + in.toUtf8() + "\"";
        else
            return in.toUtf8();
    };



    for(auto&& row : data)
    {
        for(int i = 0; i<numCol; ++i)
        {
            csvFileOut<<strOutput(row[i]);

            // Add the terminating character
            if(i != numCol-1)
                csvFileOut<<",";
            else
                csvFileOut<<"\n";
        }
    }


    return 0;
}



QVector<QStringList> CSVReaderWriter::parseCSVFile(const QString &pathToFile, QString& err)
{
    QVector<QStringList> returnVec;

    QFile geomFile(pathToFile);

    if (!geomFile.open(QIODevice::ReadOnly))
    {
        err = "Cannot find the file: " + pathToFile + "\nCheck your directory and try again.";
        return returnVec;
    }

    QStringList rowLines;
    while (!geomFile.atEnd())
    {
        QString line = geomFile.readLine();

        rowLines << line;
    }

    auto numRows = rowLines.size();
    if(numRows == 0)
    {
        err = "Error in parsing the .csv file " + pathToFile + " in " + __PRETTY_FUNCTION__;
        return returnVec;
    }

    returnVec.reserve(numRows);

    for(auto&& it: rowLines)
    {
        auto lineStr = this->parseLineCSV(it);

        returnVec.push_back(lineStr);
    }

    return returnVec;
}


QStringList CSVReaderWriter::parseLineCSV(const QString &csvString)
{
    QStringList fields;
    QString value;

    bool hasQuote = false;

    for (int i = 0; i < csvString.size(); ++i)
    {
        const QChar current = csvString.at(i);

        // Normal state
        if (hasQuote == false)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                hasQuote = true;
                value += current;
            }

            // Other character
            else
                value += current;
        }
        else if (hasQuote)
        {
            // Check for another double-quote
            if (current == '"')
            {
                if (i < csvString.size())
                {
                    // A double double-quote?
                    if (i+1 < csvString.size() && csvString.at(i+1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        hasQuote = false;
                        value += '"';
                    }
                }
            }

            // Other character
            else
                value += current;
        }
    }

    if (!value.isEmpty())
        fields.append(value.trimmed());


    // Remove quotes and whitespace around quotes
    for (int i=0; i<fields.size(); ++i)
        if (fields[i].length()>=1 && fields[i].left(1)=='"')
        {
            fields[i]=fields[i].mid(1);
            if (fields[i].length()>=1 && fields[i].right(1)=='"')
                fields[i]=fields[i].left(fields[i].length()-1);
        }

    return fields;
}

