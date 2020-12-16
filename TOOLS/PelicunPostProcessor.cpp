#include "PelicunPostProcessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"
#include "TablePrinter.h"
#include "WorkflowAppRDT.h"
#include "GeneralInformationWidget.h"
#include "REmpiricalProbabilityDistribution.h"

#include <QHeaderView>
#include <QGroupBox>
#include <QTableWidget>
#include <QFileInfo>
#include <QLabel>
#include <QDir>
#include <QStringList>
#include <QGridLayout>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QBarCategoryAxis>
#include <QStackedBarSeries>
#include <QValueAxis>
#include <QChartView>
#include <QGraphicsLayout>
#include <QPrinter>
#include <QTextCursor>
#include <QPixmap>
#include <QFontMetrics>
#include <QTextTable>
#include <QComboBox>
#include <QLineSeries>

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"
#include "FeatureTable.h"

using namespace QtCharts;

PelicunPostProcessor::PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&PelicunPostProcessor::assemblePDF);

    // Create the table that will show the Component information
    pelicunResultsTableWidget = new QTableWidget(this);
    pelicunResultsTableWidget->verticalHeader()->setVisible(false);
    pelicunResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    pelicunResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    pelicunResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    pelicunResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pelicunResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QStringList tableHeadings = {"Asset ID","Repair\nCost","Repair\nTime","Replacement\nProbability","Fatalities","Loss\nRatio"};

    pelicunResultsTableWidget->setColumnCount(tableHeadings.size());
    pelicunResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    pelicunResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Combo box to select how to sort the table
    QStringList comboBoxHeadings = {"Asset ID","Repair Cost","Repair Time","Replacement Probability","Fatalities","Loss Ratio"};
    sortComboBox = new QComboBox();
    sortComboBox->insertItems(0,comboBoxHeadings);

    sortComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    auto comboBoxLabel = new QLabel("Sorting Filter:", this);

    connect(sortComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this, &PelicunPostProcessor::sortTable);

    // Layout to display the results
    resultsGridLayout = new QGridLayout();
    resultsGridLayout->setContentsMargins(10,0,0,0);

    QGroupBox* totalsWidget = new QGroupBox("Estimated Regional Totals",this);
    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

    QLabel* estCasLabel = new QLabel("Estimated Casualties", this);
    QLabel* estLossLabel = new QLabel("Estimated Economic Losses", this);
    QLabel* relFreqLabel = new QLabel("Relative Frequency Diagram of Expected Losses", this);

    estCasLabel->setStyleSheet("font-weight: bold");
    estLossLabel->setStyleSheet("font-weight: bold");
    relFreqLabel->setStyleSheet("font-weight: bold");

    totalCasLabel = new QLabel("Casualties:", this);
    totalFatalitiesLabel = new QLabel("Fatalities:", this);
    totalLossLabel = new QLabel("Losses:", this);
    totalRepairTimeLabel = new QLabel("Repair Time [days]:", this);
    structLossLabel = new QLabel("Structural Losses:", this);
    nonStructLossLabel = new QLabel("Non-structural Losses:", this);

    totalCasValueLabel = new QLabel("", this);
    totalLossValueLabel = new QLabel("", this);
    totalRepairTimeValueLabel = new QLabel("", this);
    totalFatalitiesValueLabel = new QLabel("", this);
    structLossValueLabel = new QLabel("", this);
    nonStructLossValueLabel = new QLabel("", this);

    totalsLayout->addWidget(totalCasLabel,0,0);
    totalsLayout->addWidget(totalCasValueLabel,0,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalFatalitiesLabel,0,2);
    totalsLayout->addWidget(totalFatalitiesValueLabel,0,3,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalLossLabel,1,0);
    totalsLayout->addWidget(totalLossValueLabel,1,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalRepairTimeLabel,1,2);
    totalsLayout->addWidget(totalRepairTimeValueLabel,1,3,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(structLossLabel,2,0);
    totalsLayout->addWidget(structLossValueLabel,2,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(nonStructLossLabel,2,2);
    totalsLayout->addWidget(nonStructLossValueLabel,2,3,1,1,Qt::AlignLeft);


    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr,mapViewMainWidget);

    mapViewSubWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    mapViewSubWidget->setFixedHeight(400);

    resultsGridLayout->addWidget(estCasLabel,0,0,Qt::AlignCenter);
    resultsGridLayout->addWidget(estLossLabel,0,1,Qt::AlignCenter);
    resultsGridLayout->addWidget(totalsWidget,0,2,2,2,Qt::AlignLeft);
    resultsGridLayout->addWidget(relFreqLabel,2,2,1,2,Qt::AlignCenter);

    // Charts go here->

    resultsGridLayout->addWidget(comboBoxLabel,7,2,1,1);

    resultsGridLayout->addWidget(sortComboBox,7,3,1,1);

    resultsGridLayout->addWidget(mapViewSubWidget.get(),6,0,4,2);

    resultsGridLayout->setColumnStretch(9, 1);

}


int PelicunPostProcessor::importResults(const QString& pathToResults)
{

    // Remove old csv files in the output pathToResults
    const QFileInfo existingFilesInfo(pathToResults);

    // Get the existing files in the folder to see if we already have the record
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);

    if(existingCSVFiles.empty())
    {
        QString errMessage = "The results folder is empty";
        return -1;
    }

    QString DMResultsSheet;
    QString DVResultsSheet;
    QString EDPreultsSheet;

    for(auto&& it : existingCSVFiles)
    {
        if(it.startsWith("DM_"))
            DMResultsSheet = it;
        else if(it.startsWith("DV_"))
            DVResultsSheet = it;
        else if(it.startsWith("EDP_"))
            EDPreultsSheet = it;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> DMdata = csvTool.parseCSVFile(pathToResults + DMResultsSheet,err);

    if(!err.isEmpty() || DMdata.empty())
        return -1;


    QVector<QStringList> DVdata = csvTool.parseCSVFile(pathToResults + DVResultsSheet,err);

    if(!err.isEmpty() || DVdata.empty())
        return -1;


    QVector<QStringList> EDPdata = csvTool.parseCSVFile(pathToResults + EDPreultsSheet,err);

    if(!err.isEmpty() || EDPdata.empty())
        return -1;


    auto res1 = this->processDVResults(DVdata);

    return res1;
}


int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DVdata)
{
    if(DVdata.size() < 4)
        return -1;

    auto numHeaders = DVdata.at(0).size();

    QVector<QString> headerStrings(numHeaders);

    for(int i = 0; i<numHeaders; ++i)
    {
        QString headerStr =  DVdata.at(0).at(i)  +"-"+ DVdata.at(1).at(i)  +"-"+  DVdata.at(2).at(i)  +"-"+  DVdata.at(3).at(i);

        headerStrings[i] = headerStr;
    }

    pelicunResultsTableWidget->setRowCount(DVdata.size()-4);

    auto cumulativeStructDS1 = 0.0;
    auto cumulativeStructDS2 = 0.0;
    auto cumulativeStructDS3 = 0.0;
    auto cumulativeStructDS4 = 0.0;

    auto cumulativeNSAccDS1 = 0.0;
    auto cumulativeNSAccDS2 = 0.0;
    auto cumulativeNSAccDS3 = 0.0;
    auto cumulativeNSAccDS4 = 0.0;

    auto cumulativeNSDriftDS1 = 0.0;
    auto cumulativeNSDriftDS2 = 0.0;
    auto cumulativeNSDriftDS3 = 0.0;
    auto cumulativeNSDriftDS4 = 0.0;

    auto cumulativeinjSevLvl1 = 0.0;
    auto cumulativeinjSevLvl2 = 0.0;
    auto cumulativeinjSevLvl3 = 0.0;
    auto cumulativeinjSevLvl4 = 0.0;

    auto cumulativeRepairTime = 0.0;

    REmpiricalProbabilityDistribution theProbDist;

    // Get the buildings database
    auto theBuildingDB = theVisualizationWidget->getBuildingDatabase();

    try
    {
        int count = 0;
        for(int i = 4; i<DVdata.size(); ++i, ++count)
        {
            auto inputRow = DVdata.at(i);

            bool OK;
            auto buildingID = inputRow.at(0).toInt(&OK);

            if(!OK)
                throw QString("Could not convert the building ID " + inputRow.at(0) + " to an integer");

            auto building = theBuildingDB->getBuilding(buildingID);

            if(building.ID == -1)
                throw QString("Could not convert the building ID " + QString::number(buildingID) + " from the database");


            for(int j = 1; j<numHeaders; ++j)
            {
                building.ResultsValues.insert(headerStrings.at(j),inputRow.at(j).toDouble());
            }

            auto replacementCostVar = building.buildingAttributes.value("replacementCost",QVariant(0.0));

            auto replacementCost = objectToDouble(replacementCostVar);

            building.ID = buildingID;

            buildingsVec.push_back(building);

            // This assumes that the output from pelicun will not change
            auto IDStr = inputRow.at(0);            // ID
            auto totalRepairCost = inputRow.at(1);  // Aggregate repair cost (mean)
            auto replaceMentProb = inputRow.at(6);  // Replacement probability, i.e., repair impractical probability
            auto repairTime = inputRow.at(28);      // Aggregate repair time (mean)

            cumulativeRepairTime += objectToDouble(repairTime);

            auto StructDS1 = objectToDouble(inputRow.at(8));    // Structural losses damage state 1 (mean)
            auto StructDS2 = objectToDouble(inputRow.at(9));    // Structural losses damage state 2 (mean)
            auto StructDS3 = objectToDouble(inputRow.at(10));   // Structural losses damage state 3 (mean)
            auto StructDS4 = objectToDouble(inputRow.at(11));   // Structural losses damage state 4 (mean)

            cumulativeStructDS1 += StructDS1;
            cumulativeStructDS2 += StructDS2;
            cumulativeStructDS3 += StructDS3;
            cumulativeStructDS4 += StructDS4;

            auto NSAccDS1 = objectToDouble(inputRow.at(19));    // Non-structural acceleration sensitive losses damage state 1 (mean)
            auto NSAccDS2 = objectToDouble(inputRow.at(20));    // Non-structural acceleration sensitive losses damage state 2 (mean)
            auto NSAccDS3 = objectToDouble(inputRow.at(21));    // Non-structural acceleration sensitive losses damage state 3 (mean)
            auto NSAccDS4 = objectToDouble(inputRow.at(22));    // Non-structural acceleration sensitive losses damage state 4 (mean)

            cumulativeNSAccDS1 += NSAccDS1;
            cumulativeNSAccDS2 += NSAccDS2;
            cumulativeNSAccDS3 += NSAccDS3;
            cumulativeNSAccDS4 += NSAccDS4;

            auto NSDriftDS1 = objectToDouble(inputRow.at(24));  // Non-structural drift sensitive losses damage state 1 (mean)
            auto NSDriftDS2 = objectToDouble(inputRow.at(25));  // Non-structural drift sensitive losses damage state 2 (mean)
            auto NSDriftDS3 = objectToDouble(inputRow.at(26));  // Non-structural drift sensitive losses damage state 3 (mean)
            auto NSDriftDS4 = objectToDouble(inputRow.at(27));  // Non-structural drift sensitive losses damage state 4 (mean)

            cumulativeNSDriftDS1 += NSDriftDS1;
            cumulativeNSDriftDS2 += NSDriftDS2;
            cumulativeNSDriftDS3 += NSDriftDS3;
            cumulativeNSDriftDS4 += NSDriftDS4;

            auto injSevLvl1 = objectToDouble(inputRow.at(33));  // Injuries severity level 1 (mean)
            auto injSevLvl2 = objectToDouble(inputRow.at(38));  // Injuries severity level 2 (mean)
            auto injSevLvl3 = objectToDouble(inputRow.at(43));  // Injuries severity level 3 (mean)
            auto fatalities = objectToDouble(inputRow.at(48));  // Injuries severity level 4 (mean)

            cumulativeinjSevLvl1 += injSevLvl1;
            cumulativeinjSevLvl2 += injSevLvl2;
            cumulativeinjSevLvl3 += injSevLvl3;
            cumulativeinjSevLvl4 += fatalities;

            auto repairCost = objectToDouble(totalRepairCost);
            auto lossRatio = repairCost/replacementCost;

            theProbDist.addSample(repairCost);

            auto IDItem = new QTableWidgetItem(IDStr);
            auto RepCostItem = new QTableWidgetItem(totalRepairCost);
            auto RepProbItem = new QTableWidgetItem(replaceMentProb);
            auto RepairTimeItem = new QTableWidgetItem(repairTime);
            auto fatalitiesItem = new QTableWidgetItem(inputRow.at(48));
            auto lossRatioItem = new QTableWidgetItem(QString::number(lossRatio));

            pelicunResultsTableWidget->setItem(count,0, IDItem);
            pelicunResultsTableWidget->setItem(count,1, RepCostItem);
            pelicunResultsTableWidget->setItem(count,2, RepairTimeItem);
            pelicunResultsTableWidget->setItem(count,3, RepProbItem);
            pelicunResultsTableWidget->setItem(count,4, fatalitiesItem);
            pelicunResultsTableWidget->setItem(count,5, lossRatioItem);

            auto buildingFeature = building.buildingFeature;

            buildingFeature->attributes()->replaceAttribute("LossRatio",lossRatio);

            buildingFeature->featureTable()->updateFeature(buildingFeature);
        }
    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);

        return -1;
    }

    //  CASUALTIES
    QBarSet *casualtiesSet = new QBarSet("Casualties");

    *casualtiesSet << cumulativeinjSevLvl1 << cumulativeinjSevLvl2 << cumulativeinjSevLvl3 << cumulativeinjSevLvl4;

    this->createCasualtiesChart(casualtiesSet);

    totalCasValueLabel->setText(QString::number(casualtiesSet->sum()));
    totalFatalitiesValueLabel->setText(QString::number(casualtiesSet->at(3)));

    //  LOSSES
    QBarSet *structLossSet = new QBarSet("Structural");
    QBarSet *NSAccLossSet = new QBarSet("Non-structural Acc.");
    QBarSet *NSDriftLossSet = new QBarSet("Non-structural Drift");

    *structLossSet << cumulativeStructDS1 << cumulativeStructDS2 << cumulativeStructDS3 << cumulativeStructDS4 ;
    *NSAccLossSet << cumulativeNSAccDS1 << cumulativeNSAccDS2 << cumulativeNSAccDS3 << cumulativeNSAccDS4 ;
    *NSDriftLossSet << cumulativeNSDriftDS1 << cumulativeNSDriftDS2 << cumulativeNSDriftDS3 << cumulativeNSDriftDS4;

    this->createLossesChart(structLossSet, NSAccLossSet, NSDriftLossSet);

    auto sumStruct = structLossSet->sum();
    auto sumNonStruct = NSAccLossSet->sum() + NSDriftLossSet->sum();

    auto sumLosses = sumStruct + sumNonStruct;
    totalLossValueLabel->setText(QString::number(sumLosses));

    structLossValueLabel->setText(QString::number(sumStruct));
    nonStructLossValueLabel->setText(QString::number(sumNonStruct));

    // Repair time
    totalRepairTimeValueLabel->setText(QString::number(cumulativeRepairTime));

    this->createHistogramChart(&theProbDist);

    // Arrange the charts in a grid
    casualtiesChartView->setMinimumHeight(250);
    casualtiesChartView->setMaximumWidth(500);

    lossesChartView->setMinimumHeight(250);
    lossesChartView->setMaximumWidth(500);

    resultsGridLayout->addWidget(casualtiesChartView,1,0,5,1);
    resultsGridLayout->addWidget(lossesChartView,1,1,5,1);

    resultsGridLayout->addWidget(lossesHistogram,3,2,4,2);

    resultsGridLayout->addWidget(pelicunResultsTableWidget,8,2,1,2,Qt::AlignTop);

    return 0;
}


int PelicunPostProcessor::createCasualtiesChart(QBarSet *casualtiesSet)
{
    QBarSeries *series = new QBarSeries();
    series->append(casualtiesSet);
    series->setBarWidth(1.0);
    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
    chart->setMargins(QMargins(5,5,5,5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    QStringList categories;
    categories << "Level 1" << "Level 2" << "Level 3" << "Level 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    chart->legend()->setVisible(false);

    casualtiesChartView = new QChartView(chart);
    casualtiesChartView->setRenderHint(QPainter::Antialiasing);
    casualtiesChartView->setContentsMargins(0,0,0,0);
    casualtiesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return 0;
}


int PelicunPostProcessor::createHistogramChart(REmpiricalProbabilityDistribution* probDist)
{
    auto yValues = probDist->getRelativeFrequencyDiagram();
    auto xValues = probDist->getHistogramTicks();

    QLineSeries *series = new QLineSeries();

    for(int i = 0; i<yValues.size(); ++i)
    {
        series->append(xValues.at(i),yValues.at(i));
    }

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
    chart->setMargins(QMargins(5,5,5,5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    QValueAxis *axisX = new QValueAxis();
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);

    series->attachAxis(axisX);

    chart->legend()->setVisible(false);

    lossesHistogram = new QChartView(chart);
    lossesHistogram->setRenderHint(QPainter::Antialiasing);
    lossesHistogram->setContentsMargins(0,0,0,0);
    lossesHistogram->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    lossesHistogram->show();

    return 0;
}


int PelicunPostProcessor::createLossesChart(QBarSet *structLossSet, QBarSet *NSAccLossSet, QBarSet *NSDriftLossSet)
{
    QStackedBarSeries *series = new QStackedBarSeries();
    series->append(structLossSet);
    series->append(NSAccLossSet);
    series->append(NSDriftLossSet);
    series->setBarWidth(1.0);
//    series->setLabelsVisible(true);
//    series->setLabelsPrecision(3);
//    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
    chart->setMargins(QMargins(5,5,5,5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    QValueAxis *axisY = new QValueAxis();
    axisY->setGridLineVisible(false);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QStringList categories;
    categories << "DS = 1" << "DS = 2" << "DS = 3" << "DS = 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->setMinorGridLineVisible(false);
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    chart->legend()->setVisible(true);

    lossesChartView = new QChartView(chart);
    lossesChartView->setRenderHint(QPainter::Antialiasing);
    lossesChartView->setContentsMargins(0,0,0,0);
    lossesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return 0;
}


int PelicunPostProcessor::printToPDF(const QString& outputPath)
{
    outputFilePath = outputPath;

    theVisualizationWidget->takeScreenShot();

    return 0;
}


int PelicunPostProcessor::assemblePDF(QImage screenShot)
{
    // The printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins(25.4, 25.4, 25.4, 25.4, QPrinter::Millimeter);
    printer.setFullPage(true);
    qreal leftMargin, topMargin;
    printer.getPageMargins(&leftMargin,&topMargin,nullptr,nullptr,QPrinter::Point);
    printer.setOutputFileName(outputFilePath);

    // Create a new document
    QTextDocument* document = new QTextDocument();
    QTextCursor cursor(document);
    document->setDocumentMargin(25.4);
    document->setDefaultFont(QFont("Helvetica"));

    // Define font styles
    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setFontCapitalization(QFont::AllUppercase);
    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

    QTextCharFormat captionFormat;
    captionFormat.setFontWeight(QFont::Light);
    captionFormat.setFontPointSize(normalFormat.fontPointSize() / 2.0);
    captionFormat.setFontItalic(true);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);

    QFontMetrics normMetrics(normalFormat.font());
    auto lineSpacing = normMetrics.lineSpacing();

    // Define alignment formats
    QTextBlockFormat alignCenter;
    alignCenter.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;
    alignCenter.setAlignment(Qt::AlignCenter);

    QTextBlockFormat alignLeft;
    alignLeft.setAlignment(Qt::AlignLeft);
    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;

    cursor.movePosition(QTextCursor::Start);

    cursor.insertBlock(alignCenter);

    // Insert the simcenter logo at the top
    QImage simCenterLogo(":resources/SimCenter@1x.png");
    document->addResource(QTextDocument::ImageResource, QUrl("Logo"), simCenterLogo);
    QTextImageFormat imageFormatSimCenterLogo;
    imageFormatSimCenterLogo.setName("Logo");
    imageFormatSimCenterLogo.setWidth(250);
    imageFormatSimCenterLogo.setQuality(600);

    cursor.insertImage(imageFormatSimCenterLogo);

    cursor.insertText("\nRegional Resilience Determination Tool (RDT)\n",titleFormat);

    cursor.insertText("Results Summary\n",boldFormat);

    cursor.setBlockFormat(alignLeft);

    cursor.insertText("Employing Pelicun loss methodology to calculate seismic losses.\n",normalFormat);

    QString currentDT = "Timestamp: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n";
    cursor.insertText(currentDT,normalFormat);

    auto workflowApp = WorkflowAppRDT::getInstance();
    auto analysisName = workflowApp->getGeneralInformationWidget()->getAnalysisName();

    QString analysisNameLabel = "Analysis name: " + analysisName + "\n";
    cursor.insertText(analysisNameLabel,normalFormat);

    cursor.insertText("Estimated Regional Totals\n",boldFormat);

    QTextTableFormat tableFormat;
    tableFormat.setPadding(5.0);
    tableFormat.setCellPadding(5.0);
    tableFormat.setBorder(0.0);
    tableFormat.setAlignment(Qt::AlignVCenter);

    tableFormat.setBackground(QColor("#f0f0f0"));
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    tableFormat.setColumnWidthConstraints(constraints);

    // rows, columns, tableFormat
    QTextTable *table = cursor.insertTable(3, 4, tableFormat);

    {
        QTextTableCell cell = table->cellAt(0, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalCasLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalCasValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalFatalitiesLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalFatalitiesValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalLossValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalRepairTimeLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalRepairTimeValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(structLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(structLossValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(nonStructLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(nonStructLossValueLabel->text());
    }

    cursor.movePosition( QTextCursor::End );

    cursor.insertText("\n\n",normalFormat);

    // Ratio of the page width that is printable
    auto useablePageWidth = printer.pageRect(QPrinter::Point).width()-(1.5*leftMargin);

    QRect viewPortRect(0, mapViewMainWidget->height() - mapViewSubWidget->height(), mapViewSubWidget->width(), mapViewSubWidget->height());
    QImage cropped = screenShot.copy(viewPortRect);
    document->addResource(QTextDocument::ImageResource,QUrl("Figure1"),cropped);
    QTextImageFormat imageFormatFig1;
    imageFormatFig1.setName("Figure1");
    imageFormatFig1.setQuality(600);
    imageFormatFig1.setWidth(useablePageWidth);

    cursor.setBlockFormat(alignCenter);

    cursor.insertImage(imageFormatFig1);

    cursor.insertText("Regional map visualization.\n",captionFormat);

    auto rectFig2 = casualtiesChartView->viewport()->rect();
    QPixmap pixmapFig2(rectFig2.size());
    QPainter painterFig2(&pixmapFig2);
    painterFig2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    casualtiesChartView->render(&painterFig2, pixmapFig2.rect(), rectFig2);
    auto figure2 = pixmapFig2.toImage();

    QTextImageFormat imageFormatFig2;
    imageFormatFig2.setName("Figure2");
    imageFormatFig2.setQuality(600);
    imageFormatFig2.setWidth(400);

    document->addResource(QTextDocument::ImageResource,QUrl("Figure2"),figure2);

    cursor.insertImage(imageFormatFig2,QTextFrameFormat::InFlow);

    cursor.insertText("\nEstimated casualties.\n",captionFormat);

    auto rectFig3 = lossesChartView->viewport()->rect();
    QPixmap pixmapFig3(rectFig3.size());
    QPainter painterFig3(&pixmapFig3);
    painterFig3.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    lossesChartView->render(&painterFig3, pixmapFig3.rect(), rectFig3);
    auto figure3 = pixmapFig3.toImage();

    QTextImageFormat imageFormatFig3;
    imageFormatFig3.setName("Figure3");
    imageFormatFig3.setQuality(600);
    imageFormatFig3.setWidth(400);

    document->addResource(QTextDocument::ImageResource,QUrl("Figure3"),figure3);
    cursor.insertImage(imageFormatFig3,QTextFrameFormat::InFlow);

    cursor.insertText("\nEstimated economic losses.\n",captionFormat);


    {
        auto rectFig4 = lossesHistogram->viewport()->rect();
        QPixmap pixmapFig4(rectFig4.size());
        QPainter painterFig4(&pixmapFig4);
        painterFig4.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        lossesHistogram->render(&painterFig4, pixmapFig4.rect(), rectFig4);
        auto figure4 = pixmapFig4.toImage();

        QTextImageFormat imageFormatFig4;
        imageFormatFig4.setName("Figure4");
        imageFormatFig4.setQuality(600);
        imageFormatFig4.setWidth(400);

        document->addResource(QTextDocument::ImageResource,QUrl("Figure4"),figure4);
        cursor.insertImage(imageFormatFig4,QTextFrameFormat::InFlow);

        cursor.insertText("\nRelative frequency diagram of expected losses.\n",captionFormat);
    }

    cursor.insertText("Individual Asset Results - Sorted According to the " + sortComboBox->currentText() + "\n",boldFormat);

    TablePrinter prettyTablePrinter;
    prettyTablePrinter.printToTable(&cursor, pelicunResultsTableWidget,"Asset Results");

    document->print(&printer);


    return 0;
}


QGridLayout *PelicunPostProcessor::getResultsGridLayout() const
{
    return resultsGridLayout;
}


void PelicunPostProcessor::sortTable(int index)
{
    if(index == 0)
        pelicunResultsTableWidget->sortByColumn(index,Qt::AscendingOrder);
    else
        pelicunResultsTableWidget->sortByColumn(index,Qt::DescendingOrder);

}


