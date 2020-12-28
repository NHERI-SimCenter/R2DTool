/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Frank McKenna

#include "PeerLoginDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QCheckBox>

PeerLoginDialog::PeerLoginDialog(PeerNgaWest2Client* peerClient, QWidget *parent) : QDialog(parent),
    peerClient(peerClient)
{
    this->setWindowTitle("Log in to PEER NGA West 2 Ground Motion Database");
    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Please use your PEER credentials to login to NGA West 2 Database"), 0, 0, 1, 2);
    auto signUpLabel =
            new QLabel("If you dont have a PEER account, you can <a href=\"https://ngawest2.berkeley.edu/users/sign_up\">sign up for a new account</a>");
    signUpLabel->setTextFormat(Qt::RichText);
    signUpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    signUpLabel->setOpenExternalLinks(true);
    layout->addWidget(signUpLabel, 1, 0, 1, 2);

    layout->addWidget(new QLabel("Username"), 2, 0);
    usernameEditBox = new QLineEdit();
    layout->addWidget(usernameEditBox, 2, 1);
    QSettings settings;
    auto peerUsernameSetting = settings.value("PeerUsername");
    if(peerUsernameSetting.isValid() && !peerUsernameSetting.isNull())
        usernameEditBox->setText(peerUsernameSetting.toString());

    //Password
    layout->addWidget(new QLabel("Password"), 3, 0);
    passwordEditBox = new QLineEdit();
    passwordEditBox->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEditBox, 3, 1);

    auto peerPasswordSetting = settings.value("PeerPassword");
    if(peerPasswordSetting.isValid() && !peerPasswordSetting.isNull())
        passwordEditBox->setText(peerPasswordSetting.toString());

    //Save Password
    auto savePassword = new QCheckBox("save password");
    layout->addWidget(savePassword, 4,1);

    if(usernameEditBox->text().isEmpty())
        usernameEditBox->setFocus();
    else
        passwordEditBox->setFocus();

    auto loginButton = new QPushButton("Log In");
    layout->addWidget(loginButton, 5, 0, 1, 2);

    connect(loginButton, &QPushButton::clicked, this, [this, loginButton, savePassword](){
        loginButton->setEnabled(false);
        loginButton->setDown(true);
        QSettings settings;
        settings.setValue("PeerUsername", usernameEditBox->text());
        if(savePassword->checkState() == Qt::Checked)
            settings.setValue("PeerPassword", passwordEditBox->text());
        this->peerClient->signIn(usernameEditBox->text(), passwordEditBox->text());
    });

    connect(this->peerClient, &PeerNgaWest2Client::loginFinished, this, [this, loginButton](bool result){
        if (result)
            this->accept();
        else
        {
            loginButton->setEnabled(true);
            loginButton->setDown(false);
        }
    });
}
