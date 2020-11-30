#ifndef HBOXFORMLAYOUT_H
#define HBOXFORMLAYOUT_H
#include <QHBoxLayout>

class HBoxFormLayout : public QHBoxLayout
{
public:
    HBoxFormLayout();
    explicit HBoxFormLayout(QWidget *parent);
    void addField(const QString& labelText, QWidget* widget);
};

#endif // HBOXFORMLAYOUT_H
