/*
   Copyright (C) 2012-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kontactconfiguredialog.h"
#include <KConfig>
#include <KSharedConfig>
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
    KConfigGroup group(KSharedConfig::openConfig(), "KontactConfigureDialog");
    group.writeEntry("width", width());
    group.writeEntry("height", height());
}

void KontactConfigureDialog::emitConfigChanged()
{
    //Add code from plugins which needs to be call when we close kontact dialog config
    QDBusInterface kmailIface(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QStringLiteral("org.kde.kmail.kmail"),
                              QDBusConnection::sessionBus());
    if (kmailIface.isValid()) {
        QDBusReply<void> reply;
        if (!(reply = kmailIface.call(QStringLiteral("updateConfig"))).isValid()) {
            const QDBusError err = kmailIface.lastError();
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

QSize KontactConfigureDialog::sizeHint() const
{
    KConfigGroup group(KSharedConfig::openConfig(), "KontactConfigureDialog");
    const int width = group.readEntry("width", 800);
    const int height = group.readEntry("height", 600);
    return QSize(width, height);
}
