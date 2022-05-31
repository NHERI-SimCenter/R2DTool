#ifndef TABLENUMBERITEM_H
#define TABLENUMBERITEM_H

#include <QTableWidgetItem>
#include <QStyledItemDelegate>

#include <cmath>
#include <stdexcept>

bool is_integer(float k)
{
  return std::floor(k) == k;
}

class DoubleDelegate : public QStyledItemDelegate
{
public:
    DoubleDelegate(QObject *parent = nullptr, int decimals = 1) : QStyledItemDelegate(parent), nDecimals(decimals)
    {
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
        bool OK;
        auto number = value.toDouble(&OK);

        if(OK)
        {
            if(is_integer(number))
            {
                return locale.toString(number,'f', 0);
            }

            return locale.toString(number,'g', nDecimals);
        }
        else
        {
            return value.toString();
        }
    }

private:
    int nDecimals;
};

class TableNumberItem : public QTableWidgetItem
{
public:

    TableNumberItem(QString const & text) : QTableWidgetItem(text), numberValue(textToDouble(text))
    {

    }

    TableNumberItem(long n, QString const & text) : QTableWidgetItem(text), numberValue(n)
    {
    }

    TableNumberItem(double n, QString const & text): QTableWidgetItem(text), numberValue(n)
    {
    }

    TableNumberItem(long n) : QTableWidgetItem(QString("%1").arg(n)), numberValue(n)
    {
    }

    TableNumberItem(double n) : QTableWidgetItem(QString("%1").arg(n)), numberValue(n)
    {
    }

    bool operator < (QTableWidgetItem const & rhs) const
    {
        TableNumberItem const * r = dynamic_cast<TableNumberItem const *>(&rhs);
        if(r == nullptr)
        {
            throw std::runtime_error("Needs to be a TableNumberItem");
        }
        return numberValue < r->numberValue;
    }

    virtual double textToDouble(QString t)
    {
        // clean the beginning of the string of whatever
        // introducers
        //
        int const sp(t.indexOf(QRegExp("[0-9.]"), 0));
        if(sp > 0)
        {
            t = t.mid(sp);
        }

        // clean the end of the string of whatever postfix
        //
        int const ep(t.indexOf(QRegExp("[^0-9. ,]"), 0));
        if(ep > 0)
        {
            t = t.mid(ep);
        }

        // now remove commas and spaces
        //
        // note: replace() happens in place so we don't have to
        //       save the result in the variable itself
        //
        t.replace(QRegExp("[, ]+"), "");

        return t.toDouble();
    }

private:
    double numberValue = 0.0;
};

#endif // TABLENUMBERITEM_H
