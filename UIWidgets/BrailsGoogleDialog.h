#ifndef BRAILS_GOOGLE_DIALOG_H
#define BRAILS_GOOGLE_DIALOG_H

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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Barbaros Cetiner, Frank McKenna

#include "qcheckbox.h"
#include "qevent.h"
#include "qstyle.h"
#include <QDialog>
#include <QJsonObject>
#include <BrailsInventoryGenerator.h>
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QCursor>
#include <QDebug>

class SC_IntLineEdit;
class QLineEdit;

class BrailsGoogleDialog : public QDialog
{
	Q_OBJECT
public:
	explicit BrailsGoogleDialog(QWidget* parent);
	void setData(BrailsData&);

signals:

public slots:
	void startBrails(void);
	void disableNumBuildings(bool checked);
	void handleBrailsFinished();

private:
	SC_IntLineEdit* numBuildings;
	QCheckBox numBldgCheckbox;
	bool getAllBuildings;
	SC_IntLineEdit* seed;
	QLabel* seedLabel;
	BrailsData brailsData;
	QLineEdit* apiKey;
};

class ClickableLabel : public QLabel {
	Q_OBJECT
public:
	explicit ClickableLabel(const QString& text = "", QWidget* parent = nullptr) : QLabel(text, parent) {}

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent* event) override {
		if (event->button() == Qt::LeftButton) {
			emit clicked();
		}
		QLabel::mousePressEvent(event);
	}
};

class HelpWidget : public QWidget {
	Q_OBJECT
public:
	HelpWidget(QWidget* parent = nullptr) : QWidget(parent) {
		QHBoxLayout* layout = new QHBoxLayout(this);

		helpIcon = new ClickableLabel("", this);
		helpIcon->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxQuestion));
		helpIcon->setToolTip("Click for help on getting a Google API Key");
		helpIcon->setCursor(Qt::PointingHandCursor);

		connect(helpIcon, &ClickableLabel::clicked, this, &HelpWidget::showHelpMessage);

		layout->addWidget(helpIcon);
	}

private slots:
	void showHelpMessage() {
		QMessageBox::question(this, "Help", "If you do not have a Google API key, please follow the instructions <a href='https://developers.google.com/maps/documentation/embed/get-api-key'>here</a>", QMessageBox::Ok);
	}

private:
	ClickableLabel* helpIcon;
};

#endif // BRAILS_GOOGLE_DIALOG_H
