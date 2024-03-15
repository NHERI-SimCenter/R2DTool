#include "IntensityMeasureWidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QJsonObject>

#include <sstream>

IntensityMeasureWidget::IntensityMeasureWidget(IntensityMeasure &im, QWidget *parent): SimCenterAppWidget(parent), m_intensityMeasure(im)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    imGroupBox = new QGroupBox(this);
    imGroupBox->setTitle("Intensity Measure(s)");
    imGroupBox->setContentsMargins(0,0,0,0);

    //imGroupBox->setMinimumWidth(400);

    QGridLayout* gridLayout = new QGridLayout(imGroupBox);
    imGroupBox->setLayout(gridLayout);

//    typeLabel = new QLabel(tr("Type Test:"),this);
    PGACheckBox = new QCheckBox("PGA (g)");
    PGACheckBox->setChecked(true);
    selectedIMTypes->append("PGA");
    SACheckBox = new QCheckBox("SA (g)");
    PGVCheckBox = new QCheckBox("PGV (cm/s)");

//    m_typeBox = new QComboBox(this);
//    m_typeBox->addItem("Spectral Accelerations (SA)", "SA");
//    m_typeBox->addItem("Peak Ground Acceleration (PGA)", "PGA");
    im.setType("Spectral Accelerations (SA)");
//    m_typeBox->setCurrentIndex(0);
    //m_typeBox->setMaximumWidth(450);
//    SACheckBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    periodLabel = new QLabel(tr("Periods:"),this);

    // Set a validator to allow only numbers, periods, and spaces
    QRegExp regExpAllow("^([1-9][0-9]*|[1-9]*\\.[0-9]*|0\\.[0-9]*)*(([ ]*,[ ]*){0,1}([[1-9]*\\.[0-9]*|[1-9][0-9]*|0\\.[0-9]*))*");
    LEValidator = new QRegExpValidator(regExpAllow,this);

    periodsLineEdit = new QLineEdit(this);
    periodsLineEdit->setText("0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 7.5, 10.0");
    periodsLineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    QList<double> periodArray = {0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 7.5, 10.0};
    im.setPeriods(periodArray);
    SAperiods = periodArray;

    periodLabel->hide();
    periodsLineEdit->hide();


//    QObject::connect(this->SACheckBox, &QCheckBox::stateChanged, [&](int state){
//        periodsLineEdit->setEnabled(state == Qt::Checked);
//    });

    // Intensity measure levels
    imtLevelLabel = new QLabel(tr("IM Levels (min, max, num):"),this);
    imtLevelLineEdit = new QLineEdit(this);
    imtLevelLineEdit->setText("0.01,10,100");
    scaleLabel = new QLabel(tr("Scale:"),this);
    scaleBox = new QComboBox(this);
    scaleBox->addItem("Log");
    scaleBox->addItem("Linear");
    connect(this->scaleBox, &QComboBox::currentTextChanged,
            &this->m_intensityMeasure, &IntensityMeasure::setImtScale);
    scaleBox->setCurrentText("Log");
    m_intensityMeasure.setImtScale("Log");
    imtLevelLabel->hide();
    imtLevelLineEdit->hide();
    scaleBox->hide();
    scaleLabel->hide();

    // Intensity measure trucation levels
    imtTrucLabel = new QLabel(tr("Truncation:"),this);
    imtTrucBox = new QDoubleSpinBox(this);
    imtTrucBox->setRange(0, 5.0);
    imtTrucBox->setDecimals(2);
    imtTrucBox->setSingleStep(0.01);
    imtTrucBox->setValue(m_intensityMeasure.getImtTruc()); // set initial values
    imtTrucLabel->hide();
    imtTrucBox->hide();

    gridLayout->addWidget(PGACheckBox, 0, 0, 1, 1);
    gridLayout->addWidget(SACheckBox, 1, 0, 1, 1);
//    gridLayout->addWidget(m_typeBox,0,1);
    gridLayout->addWidget(periodLabel,1,1, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(periodsLineEdit,1,2, 1, 3);
    gridLayout->addWidget(PGVCheckBox, 2, 0, 1, 1);
    gridLayout->setColumnStretch(2,1);
//    gridLayout->addWidget(imtLevelLabel,1,0);
//    gridLayout->addWidget(imtLevelLineEdit,1,1);
//    gridLayout->addWidget(scaleLabel,1,2);
//    gridLayout->addWidget(scaleBox,1,3);
//    gridLayout->addWidget(imtTrucLabel,1,4);
//    gridLayout->addWidget(imtTrucBox,1,5);
    layout->addWidget(imGroupBox);
    this->setLayout(layout);

    this->setupConnections();
}


void IntensityMeasureWidget::setupConnections()
{
    connect(this->m_typeBox, &QComboBox::currentTextChanged,
            &this->m_intensityMeasure, &IntensityMeasure::setType);

    connect(&this->m_intensityMeasure, &IntensityMeasure::typeChanged,
            this->m_typeBox, &QComboBox::setCurrentText);

    connect(this->periodsLineEdit, &QLineEdit::textChanged, this, &IntensityMeasureWidget::checkPeriodsValid);
    connect(this->periodsLineEdit, &QLineEdit::editingFinished, this, &IntensityMeasureWidget::commitPeriods);
    //    connect(this->periodsLineEdit, &QLineEdit::inputRejected, this->periodsLineEdit, &QLineEdit::undo);

    connect(this->PGACheckBox, &QCheckBox::stateChanged, this, &IntensityMeasureWidget::handleTypeChanged);
    connect(this->SACheckBox, &QCheckBox::stateChanged, this, &IntensityMeasureWidget::handleTypeChanged);
    connect(this->PGVCheckBox, &QCheckBox::stateChanged, this, &IntensityMeasureWidget::handleTypeChanged);

    // send imtLevels
    connect(this->imtLevelLineEdit, SIGNAL(textChanged(QString)),&this->m_intensityMeasure, SLOT(setImtLevels(QString)));

    // send trucation levels
    connect(this->imtTrucBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_intensityMeasure, &IntensityMeasure::setImtTruc);

//    periodsLineEdit->setReadOnly(true);
//    periodsLineEdit->setEnabled(false);
//    periodsLineEdit->setStyleSheet("QLineEdit {background-color: gray;}");
}


QString IntensityMeasureWidget::checkPeriodsValid(const QString& input) const
{

    QString validInput = input;

    if(validInput.isEmpty())
        return validInput;

    int pos = 0;
    if(LEValidator->validate(const_cast<QString&>(input), pos) != 1)
    {
        validInput = QStringRef(&input, 0, pos-1).toString();

        qDebug()<<"pos"<<pos<<" : "<<validInput;
        periodsLineEdit->setText(validInput);
    }

    return validInput;
}


void IntensityMeasureWidget::commitPeriods()
{
    auto LEtext = periodsLineEdit->text();

    if(LEtext.isEmpty())
        return;

    auto inputStr = this->checkPeriodsValid(LEtext);

    QList<double> periodArray;

    // Remove any white space from the string
    inputStr.remove(" ");

    // Split the incoming text into the parts delimited by commas
    std::vector<std::string> subStringVec;

    // Create string stream from the string
    std::stringstream s_stream(inputStr.toStdString());

    // Split the input string to substrings at the comma
    while(s_stream.good())
    {
        std:: string subString;
        getline(s_stream, subString, ',');

        if(subString.empty())
            continue;

        subStringVec.push_back(subString);
    }

    for(auto&& subStr : subStringVec)
    {

        // Convert strings into numbers
        auto period = std::stod(subStr);

        periodArray.push_back(period);
    }

    m_intensityMeasure.setPeriods(periodArray);
    SAperiods = periodArray;
}


void IntensityMeasureWidget::handleTypeChanged(int state)
{
    selectedIMTypes->clear();
    if (SACheckBox->isChecked()){
        periodLabel->show();
        periodsLineEdit->show();
        selectedIMTypes->append("SA");
    } else {
        periodLabel->hide();
        periodsLineEdit->hide();
    }

    if (PGVCheckBox->isChecked()){
        selectedIMTypes->append("PGV");
    }

    if (PGACheckBox->isChecked()){
        selectedIMTypes->append("PGA");
    }
    emit IMSelectionChanged(selectedIMTypes);
}

QStringList* IntensityMeasureWidget::getSelectedIMTypes(void){
    return selectedIMTypes;
}

void IntensityMeasureWidget::handleIntensityMeasureLevels(const QString sourceType)
{
    if (sourceType.compare("OpenQuake Classical")==0)
    {
        imGroupBox->show();
        // users are expected to give intensity measure levels
        imtLevelLabel->show();
        imtLevelLineEdit->show();
        scaleBox->show();
        scaleLabel->show();
        imtTrucLabel->show();
        imtTrucBox->show();
    }
    else if (sourceType.compare("OpenQuake User-Specified")==0)
    {
        imGroupBox->hide();
    }
    else
    {
        imGroupBox->show();
        imtLevelLabel->hide();
        imtLevelLineEdit->hide();
        scaleBox->hide();
        scaleLabel->hide();
        imtTrucLabel->hide();
        imtTrucBox->hide();
    }
}


QComboBox *IntensityMeasureWidget::typeBox() const
{
    return m_typeBox;
}

bool IntensityMeasureWidget::outputToJSON(QJsonObject &jsonObject)
{
//    if (selectedIMTypes->size()==1){
//        jsonObject.insert("Type", selectedIMTypes->at(0));
//        if (selectedIMTypes->contains("SA")){
//            QJsonArray arrayPeriods;
//            for(auto&& it:SAperiods){
//                arrayPeriods.append(it);
//            }
//            jsonObject.insert("Period", arrayPeriods);
//        } else {
//            jsonObject.insert("Period", 0);
//        }
//    } else {
//        foreach(QString IMtype, &selectedIMTypes){
//            QJsonObject IMobj;
//            IMobj.insert("Type", IMtype);
//            if (IMtype.compare("SA")==0){
//                QJsonArray arrayPeriods;
//                for(auto&& it:SAperiods){
//                    arrayPeriods.append(it);
//                }
//                IMobj.insert("Period", arrayPeriods);
//            } else {
//                IMobj.insert("Period", 0);
//            }
//        }
//    }
    if (selectedIMTypes->size()==0){
        errorMessage("Ground Motion Models: At least one intensity measure needs to be selected." );
        return false;
    }
    for(int i = 0; i < selectedIMTypes->size(); i++){
        QString IMtype = selectedIMTypes->at(i);
        QJsonObject IMobj;
        IMobj.insert("Type", IMtype);
        if (IMtype.compare("SA")==0){
            QJsonArray arrayPeriods;
            for(auto&& it:SAperiods){
                arrayPeriods.append(it);
            }
            IMobj.insert("Periods", arrayPeriods);
        } else {
            IMobj.insert("Period", 0);
        }
        jsonObject[IMtype] = IMobj;
    }
    if (selectedIMTypes->size()==1){
        jsonObject = jsonObject[selectedIMTypes->at(0)].toObject();
    }


    return true;
}
