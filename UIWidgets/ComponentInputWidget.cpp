#include "ComponentInputWidget.h"
#include "AssetInputDelegate.h"
#include "CSVReaderWriter.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFileInfo>
#include <QJsonObject>

// Std library headers
#include <string>
#include <algorithm>


ComponentInputWidget::ComponentInputWidget(QWidget *parent, QString type) : SimCenterAppWidget(parent), componentType(type)
{
    label1 = "Load information from a CSV file";
    label2 = "Enter the IDs of one or more " + componentType.toLower() + " to analyze. Leave blank to analyze all " + componentType.toLower() + "."
                                                                                                                                                "\nDefine a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    label3 = QStringRef(&componentType, 0, componentType.length()) + " Information";

    pathToComponentInfoFile = "NULL";
    componentGroupBox = nullptr;
    this->createComponentsBox();
}


ComponentInputWidget::~ComponentInputWidget()
{

}


void ComponentInputWidget::loadComponentData(void)
{

    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInfoFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();

    if(pathToComponentInfoFile.compare("NULL") == 0)
        return;

    // Check if the directory exists
    QFile file(pathToComponentInfoFile);
    if (!file.exists())
    {
        QString errMsg = "Cannot find the file: "+ pathToComponentInfoFile + "\n" +"Check your directory and try again.";
        this->userMessageDialog(errMsg);
        return;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToComponentInfoFile,err);

    if(!err.isEmpty())
    {
        this->userMessageDialog(err);
        return;
    }

    if(data.empty())
        return;

    // Get the header file
    QStringList tableHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();
    auto numCols = tableHeadings.size();

    componentTableWidget->clear();
    componentTableWidget->setColumnCount(numCols);
    componentTableWidget->setRowCount(numRows);
    componentTableWidget->setHorizontalHeaderLabels(tableHeadings);

    // Fill in the cells
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStringList = data[i];

        if(rowStringList.size() != numCols)
        {
            this->userMessageDialog("Error, the number of items in row " + QString::number(i+1) + " does not equal number of headings in the file");
            return;
        }

        for(int j = 0; j<numCols; ++j)
        {
            auto cellData = rowStringList[j];

            auto item = new QTableWidgetItem(cellData);

            componentTableWidget->setItem(i,j, item);
        }
    }

    componentInfoText->show();
    componentTableWidget->show();

    emit componentDataLoaded();

    return;
}


void ComponentInputWidget::chooseComponentInfoFileDialog(void)
{
    pathToComponentInfoFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));

    // Return if the user cancels
    if(pathToComponentInfoFile.isEmpty())
    {
        pathToComponentInfoFile = "NULL";
        return;
    }

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(pathToComponentInfoFile);

    return;
}


QTableWidget *ComponentInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


QGroupBox* ComponentInputWidget::getComponentsWidget(void)
{
    if(componentGroupBox == nullptr)
        this->createComponentsBox();

    return componentGroupBox;
}


void ComponentInputWidget::createComponentsBox(void)
{

    componentGroupBox = new QGroupBox(componentType);
    componentGroupBox->setFlat(true);

    QGridLayout* gridLayout = new QGridLayout();
    componentGroupBox->setLayout(gridLayout);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* topText = new QLabel();
    topText->setText(label1);

    QLabel* pathText = new QLabel();
    pathText->setText("Import Path:");

    componentFileLineEdit = new QLineEdit();
    componentFileLineEdit->setMaximumWidth(750);
    componentFileLineEdit->setMinimumWidth(400);
    componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    QPushButton *loadFileButton = new QPushButton();
    loadFileButton->setText(tr("Load File"));
    loadFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    connect(loadFileButton,SIGNAL(clicked()),this,SLOT(loadComponentData()));

    // Add a horizontal spacer after the browse and load buttons
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel* selectComponentsText = new QLabel();
    selectComponentsText->setText(label2);

    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&ComponentInputWidget::handleComponentSelection);

    QPushButton *selectComponentsButton = new QPushButton();
    selectComponentsButton->setText(tr("Select"));
    selectComponentsButton->setMaximumWidth(150);

    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));

    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);

    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));

    // Text label for Component information
    componentInfoText = new QLabel(label3);
    componentInfoText->setStyleSheet("font-weight: bold; color: black");
    componentInfoText->hide();

    // Create the table that will show the Component information
    componentTableWidget = new QTableWidget();
    componentTableWidget->hide();
    componentTableWidget->setToolTip("Component details");
    componentTableWidget->verticalHeader()->setVisible(false);
    componentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    componentTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    componentTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);

    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addItem(smallVSpacer,0,0,1,5);
    gridLayout->addWidget(topText,1,0,1,5);
    gridLayout->addWidget(pathText,2,0);
    gridLayout->addWidget(componentFileLineEdit,2,1);
    gridLayout->addWidget(browseFileButton,2,2);
    gridLayout->addWidget(loadFileButton,2,3);
    gridLayout->addItem(hspacer, 2, 4);
    gridLayout->addWidget(selectComponentsText, 3, 0, 1, 4);
    gridLayout->addWidget(selectComponentsLineEdit, 4, 0, 1, 2);
    gridLayout->addWidget(selectComponentsButton, 4, 2);
    gridLayout->addWidget(clearSelectionButton, 4, 3);

    gridLayout->addItem(smallVSpacer,5,0,1,5);
    gridLayout->addWidget(componentInfoText,6,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget, 7, 0, 1, 5);
    gridLayout->setRowStretch(8, 1);
    //    gridLayout->addItem(vspacer, 8, 0);
    this->setLayout(gridLayout);
}


void ComponentInputWidget::selectComponents(void)
{
    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);
    }
}


void ComponentInputWidget::handleComponentSelection(void)
{

    auto nRows = componentTableWidget->rowCount();

    if(nRows == 0)
        return;

    // Get the ID of the first and last component
    bool OK;
    auto firstID = componentTableWidget->item(0,0)->data(0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->userMessageDialog(msg);
        return;
    }

    auto lastID = componentTableWidget->item(nRows-1,0)->data(0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->userMessageDialog(msg);
        return;
    }

    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    // First check that all of the selected IDs are within range
    for(auto&& it : selectedComponentIDs)
    {
        if(it<firstID || it>lastID)
        {
            QString msg = "The component ID " + QString::number(it) + " is out of range of the components provided";
            this->userMessageDialog(msg);
            return;
        }
    }

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
        componentTableWidget->setRowHidden(i,true);

    // Unhide the selected rows
    for(auto&& it : selectedComponentIDs)
        componentTableWidget->setRowHidden(it - firstID,false);

    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " " + componentType.toLower() + " are selected for analysis";
    this->userMessageDialog(msg);
}


void ComponentInputWidget::clearComponentSelection(void)
{
    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
    {
        componentTableWidget->setRowHidden(i,false);
    }

    componentFileLineEdit->clear();
}


void ComponentInputWidget::setLabel1(const QString &value)
{
    label1 = value;
}


void ComponentInputWidget::setLabel2(const QString &value)
{
    label2 = value;
}


void ComponentInputWidget::setLabel3(const QString &value)
{
    label3 = value;
}


void ComponentInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void ComponentInputWidget::setComponentType(const QString &value)
{
    componentType = value;
}


void ComponentInputWidget::insertSelectedComponent(const int ComponentID)
{
    selectComponentsLineEdit->insertSelectedCompoonent(ComponentID);
}


int ComponentInputWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString ComponentInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInfoFile;
}


void ComponentInputWidget::testFileLoad(QString& path)
{
    pathToComponentInfoFile = path;
    componentFileLineEdit->setText(path);
    this->loadComponentData();

}

bool ComponentInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
   qDebug() << "Component OutputAPPDATAtoJSON";
    jsonObject["Application"]=componentType;

    QJsonObject data;
    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()) {
        data["sourceFile"]=componentFile.fileName();
        data["pathToSource"]=componentFile.path();
    } else {
        data["sourceFile"]=QString("None");
        data["pathToSource"]=QString("");
        return false;
    }

    jsonObject["ApplicationData"] = data;

    return true;
}


bool ComponentInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

}


bool ComponentInputWidget::outputToJSON(QJsonObject &rvObject)
{

}


bool ComponentInputWidget::inputFromJSON(QJsonObject &rvObject)
{

}


bool ComponentInputWidget::copyFiles(QString &destName)
{

}
