#ifndef NGAW2CONVERTER_H
#define NGAW2CONVERTER_H

#include <QJsonObject>

class NGAW2Converter
{
public:
    NGAW2Converter();

    int convertToSimCenterEvent(const QString& pathToOutputDirectory, const QJsonObject& NGA2Results, QString& errorMsg, QJsonObject* createdRecords);

    int parseNGAW2SearchResults(const QString& filesDirectoryPath, QJsonObject& resultsJson, QString& errorMsg);

private:
    int convertRecordToJson(const QString& inputFile, QJsonObject& recordJson, QString& errorMsg);

    double getPGA(const QJsonArray* timeHistory);

    bool directionH1;
    bool directionH2;
    bool directionVert;

};

#endif // NGAW2CONVERTER_H
