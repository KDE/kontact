/*
   SPDX-FileCopyrightText: 2012-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kontactconfiguredialog.h"
#include "kontact_debug.h"
#include <KConfig>
#include <KSharedConfig>
#include <QDBusInterface>
#include <QDBusReply>
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
    // Add code from plugins which needs to be call when we close kontact dialog config
    QDBusInterface kmailIface(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QStringLiteral("org.kde.kmail.kmail"), QDBusConnection::sessionBus());
    if (kmailIface.isValid()) {
        QDBusReply<void> reply;
        if (!(reply = kmailIface.call(QStringLiteral("updateConfig"))).isValid()) {
            const QDBusError err = kmailIface.lastError();
            qCritical() << "Communication problem with KMail. "
                        << "Error message was:" << err.name() << ": \"" << err.message() << "\"";
        }
    }
    QDBusInterface knotesIface(QStringLiteral("org.kde.kontact"),
                               QStringLiteral("/KNotes"),
                               QStringLiteral("org.kde.kontact.KNotes"),
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
