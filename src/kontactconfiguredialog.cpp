/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "kontactconfiguredialog.h"
#include <KHelpClient>

#include <QDBusReply>
#include <QDBusInterface>
#include "kontact_debug.h"
#include <QPushButton>

using namespace Kontact;

KontactConfigureDialog::KontactConfigureDialog(QWidget *parent)
    : KSettings::Dialog(parent)
{
    setFaceType(Tree);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &KontactConfigureDialog::slotOk);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KontactConfigureDialog::slotApply);
}

KontactConfigureDialog::~KontactConfigureDialog()
{
}

void KontactConfigureDialog::emitConfigChanged()
{
    //Add code from plugins which needs to be call when we close kontact dialog config
    QDBusInterface kmailIface(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QStringLiteral("org.kde.kmail.kmail"),
                              QDBusConnection::sessionBus());
    if (kmailIface.isValid()) {
        QDBusReply<void> reply;
        if (!(reply = kmailIface.call(QStringLiteral("updateConfig"))).isValid()) {
            QDBusError err = kmailIface.lastError();
            qCritical() << "Communication problem with KMail. "
                        << "Error message was:" << err.name() << ": \"" << err.message() << "\"";
        }
    }
    QDBusInterface knotesIface(QStringLiteral("org.kde.kontact"), QStringLiteral("/KNotes"), QStringLiteral("org.kde.kontact.KNotes"),
                               QDBusConnection::sessionBus());
    if (knotesIface.isValid()) {
        QDBusReply<void> reply;
        if (!(reply = knotesIface.call(QStringLiteral("updateConfig"))).isValid()) {
            const QDBusError err = knotesIface.lastError();
            qCritical() << "Communication problem with KNotes. "
                        << "Error message was:" << err.name() << ": \"" << err.message() << "\"";
        }

    }
}

void KontactConfigureDialog::slotHelpClicked()
{
    KHelpClient::invokeHelp(QStringLiteral("main-config"), QStringLiteral("kontact"));
}

void KontactConfigureDialog::slotApply()
{
    slotApplyClicked();
    emitConfigChanged();
}

void KontactConfigureDialog::slotOk()
{
    slotOkClicked();
    emitConfigChanged();
}

