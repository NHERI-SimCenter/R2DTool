#include "ComponentInputWidget.h"

#include "CSVReaderWriter.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>

// Std library headers
#include <sstream>
#include <string>
#include <algorithm>


ComponentInputWidget::ComponentInputWidget(QWidget *parent, QString type) : SimCenterAppWidget(parent), componentType(type)
{
    label1 = "Load information from a CSV file";
    label2 = "Enter the IDs of one or more " + componentType.toLower() + " to analyze. Leave blank to analyze all " + componentType.toLower() + "."
             "\nDefine a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    label3 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";

    pathToComponentInfoFile = "NULL";
    componentGroupBox = nullptr;
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

    selectComponentsLineEdit = new QLineEdit();
    selectComponentsLineEdit->setMaximumWidth(1000);
    selectComponentsLineEdit->setMinimumWidth(400);
    selectComponentsLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    selectComponentsLineEdit->setPlaceholderText("e.g., 1, 3, 5-10, 12");

    // Create a regExp validator to make sure only '-' & ',' & ' ' & numbers are input
    QRegExp LERegExp ("((([1-9][0-9]*)|([1-9][0-9]*-[1-9][0-9]*))[ ]*,[ ]*)*([[1-9][0-9]*-[1-9][0-9]*|[1-9][0-9]*)");
    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);
    selectComponentsLineEdit->setValidator(LEValidator);

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
    componentTableWidget->verticalHeader()->setVisible(false);
    componentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    componentTableWidget->hide();

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

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
    gridLayout->addItem(vspacer, 8, 0);
}


void ComponentInputWidget::handleComponentSelection(void)
{
    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
        componentTableWidget->setRowHidden(i,true);

    // Unhide the selected rows
    for(auto&& it : selectedComponentIDs)
        componentTableWidget->setRowHidden(it-1,false);

    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " " + componentType.toLower() + " are selected for analysis";
    this->userMessageDialog(msg);
}


std::set<int>& ComponentInputWidget::getSelectedComponentIDs()
{
    return selectedComponentIDs;
}


void ComponentInputWidget::clearComponentSelection(void)
{
    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
    {
        componentTableWidget->setRowHidden(i,false);
    }

    selectedComponentIDs.clear();
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


void ComponentInputWidget::selectComponents()
{
    auto inputText = selectComponentsLineEdit->text();

    // Quick return if the input text is empty
    if(inputText.isEmpty())
        return;

    // Remove any white space from the string
    inputText.remove(" ");

    // Split the incoming text into the parts delimited by commas
    std::vector<std::string> subStringVec;

    // Create string stream from the string
    std::stringstream s_stream(inputText.toStdString());

    // Split the input string to substrings at the comma
    while(s_stream.good()) {
        std:: string subString;
        getline(s_stream, subString, ',');
        subStringVec.push_back(subString);
    }

    auto nRows = componentTableWidget->rowCount();

    QString msg = "Error: the provided asset ID is out of bounds";

    // Check for the case where the IDs are given as a range
    std::string dashDelim = "-";

    for(auto&& subStr : subStringVec)
    {
        // Handle the case where there is a range of assets separated by a '-'
        if (subStr.find(dashDelim) != std::string::npos)
        {
            auto pos = subStr.find(dashDelim);
            // Get the strings on either side of the '-' character
            std::string intStart = subStr.substr(0, pos);
            std::string intEnd = subStr.substr(pos + dashDelim.length());

            // Convert them into integers
            auto IDStart = std::stoi(intStart);
            auto IDEnd = std::stoi(intEnd);

            // Make sure that the end integer is greater than the first
            if(IDStart>IDEnd)
            {
                QString err = "Error in the range of asset IDs provided in the Component asset selection box";
                this->userMessageDialog(err);
                continue;
            }

            if(IDEnd-1 >= nRows)
            {
                this->userMessageDialog(msg);
                return;
            }

            // Add the IDs to the set
            for(int ID = IDStart; ID<=IDEnd; ++ID)
                selectedComponentIDs.insert(ID);
        }
        else // Asset ID is given individually
        {
            auto ID = std::stoi(subStr);

            if(ID-1 >= nRows)
            {
                this->userMessageDialog(msg);
                return;
            }

            selectedComponentIDs.insert(ID);
        }
    }

    this->handleComponentSelection();
}


void ComponentInputWidget::insertSelectedComponent(const int ComponentID)
{
    selectedComponentIDs.insert(ComponentID);
}


int ComponentInputWidget::numberComponentsSelected(void)
{
    return selectedComponentIDs.size();
}

void ComponentInputWidget::testFileLoad(QString& path)
{
    pathToComponentInfoFile = path;
    componentFileLineEdit->setText(path);
    this->loadComponentData();

}
