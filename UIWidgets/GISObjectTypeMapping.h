#ifndef GISOBJECTTYPEMAPPING_H
#define GISOBJECTTYPEMAPPING_H

#include <QString>
#include "SimpleMarkerSymbol.h"

inline QString getTextFromMarkerSymbolType(Esri::ArcGISRuntime::SimpleMarkerSymbolStyle style)
{
    switch(static_cast<int>(style))
    {
    case 0: return "Circle";
    case 1: return "Cross";
    case 2: return "Diamond";
    case 3: return "Square";
    case 4: return "Triangle";
    case 5: return "X";

    default: return {};
    }
}

inline int getMarkerSymbolTypeFromText(QString style)
{
    if(style.compare("Circle") == 0)
        return 0;
    else if(style.compare("Cross") == 0)
        return 1;
    else if(style.compare("Diamond") == 0)
        return 2;
    else if(style.compare("Square") == 0)
        return 3;
    else if(style.compare("Triangle") == 0)
        return 4;
    else if(style.compare("X") == 0)
        return 5;
    else
        return -1;
}


inline QString getTextFromLineSymbolType(Esri::ArcGISRuntime::SimpleLineSymbolStyle style)
{
    switch(static_cast<int>(style))
    {
    case 0: return "Dash";
    case 1: return "Dash Dot";
    case 2: return "Dash Dot Dot";
    case 3: return "Dot";
    case 4: return "Null";
    case 5: return "Solid";

    default: return {};
    }
}


inline int getLineSymbolTypeFromText(QString style)
{
    if(style.compare("Dash") == 0)
        return 0;
    else if(style.compare("Dash Dot") == 0)
        return 1;
    else if(style.compare("Dash Dot Dot") == 0)
        return 2;
    else if(style.compare("Dot") == 0)
        return 3;
    else if(style.compare("Null") == 0)
        return 4;
    else if(style.compare("Solid") == 0)
        return 5;
    else
        return -1;
}


inline QString getTextFromFillSymbolType(Esri::ArcGISRuntime::SimpleFillSymbolStyle style)
{
    switch(static_cast<int>(style))
    {
    case 0: return "Backward Diagonal";
    case 1: return "Cross";
    case 2: return "Diagonal Cross";
    case 3: return "Forward Diagonal";
    case 4: return "Horizontal";
    case 5: return "Null";
    case 6: return "Solid";
    case 7: return "Vertical";

    default: return {};
    }
}


inline int getFillSymbolTypeFromText(QString style)
{
    if(style.compare("Backward Diagonal") == 0)
        return 0;
    else if(style.compare("Cross") == 0)
        return 1;
    else if(style.compare("Diagonal Cross") == 0)
        return 2;
    else if(style.compare("Forward Diagonal") == 0)
        return 3;
    else if(style.compare("Horizontal") == 0)
        return 4;
    else if(style.compare("Null") == 0)
        return 5;
    else if(style.compare("Solid") == 0)
        return 6;
    else if(style.compare("Vertical") == 0)
        return 7;
    else
        return -1;
}




#endif // GISOBJECTTYPEMAPPING_H
