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

// Written by: Stevan Gavrilovic

#include "CSVReaderWriter.h"

#include <QVector>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <iomanip>

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
        auto newStr = in;

        newStr = newStr.replace("\"","\"\"");

        if(newStr.contains(','))
            return "\"" + newStr.toUtf8() + "\"";
        else
         return newStr.toUtf8();
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

int CSVReaderWriter::saveCSVFile(const QVector<QStringList>& data, const QString& pathToFile, QString& err, int precision)
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
        auto newStr = in;

        newStr = newStr.replace("\"","\"\"");
        QString str;
        if(newStr.contains(',')){
            str = "\"" + newStr.toUtf8() + "\"";
        }
        else
            str = newStr.toUtf8();
        // Check if str is a number, if yes, convert to scientific notation with 15 significant digits
        bool isdouble;
        double value = str.toDouble(&isdouble);
        if (isdouble){
            QString formattedValue = QString::number(value, 'e', 15);  // 15 significant digits, trailing zeros are omitted
            return formattedValue;
        } else {
            return str;
        }

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
        err = "Error in parsing the .csv file " + pathToFile + " in CVSReaderWriter::parseCSVFile";
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

