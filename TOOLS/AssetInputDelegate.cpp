#include "AssetInputDelegate.h"

#include <QRegExpValidator>

#include <sstream>

AssetInputDelegate::AssetInputDelegate()
{
    this->setMaximumWidth(1000);
    this->setMinimumWidth(400);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    this->setPlaceholderText("e.g., 1, 3, 5-10, 12");

    // Create a regExp validator to make sure only '-' & ',' & ' ' & numbers are input
    QRegExp LERegExp ("((([0-9]*)|([0-9]+-[1-9][0-9]*))[ ]*,[ ]*)*([[0-9]+-[1-9][0-9]*|[0-9]*)");
    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);
    this->setValidator(LEValidator);
}


int AssetInputDelegate::size()
{
    selectedComponentIDs.size();
}


void AssetInputDelegate::clear()
{
    selectedComponentIDs.clear();
}


void AssetInputDelegate::insertSelectedCompoonent(const int id)
{
    selectedComponentIDs.insert(id);
}


void AssetInputDelegate::selectComponents()
{
    auto inputText = this->text();

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

        if(!subString.empty())
            subStringVec.push_back(subString);
    }


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
                throw err;
                continue;
            }

            // Add the IDs to the set
            for(int ID = IDStart; ID<=IDEnd; ++ID)
                selectedComponentIDs.insert(ID);
        }
        else // Asset ID is given individually
        {
            auto ID = std::stoi(subStr);

            selectedComponentIDs.insert(ID);
        }
    }

    emit componentSelectionComplete();
}


std::set<int> AssetInputDelegate::getSelectedComponentIDs() const
{
    return selectedComponentIDs;
}

