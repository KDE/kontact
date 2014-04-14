/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include <KDebug>

#include <QDBusReply>
#include <QDBusInterface>

using namespace Kontact;

KontactConfigureDialog::KontactConfigureDialog( QWidget *parent )
    : KSettings::Dialog( parent )
{
    connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
    connect( this, SIGNAL(applyClicked()), SLOT(slotApply()) );
}

KontactConfigureDialog::~KontactConfigureDialog()
{
}

void KontactConfigureDialog::emitConfigChanged()
{
    //Add code from plugins which needs to be call when we close kontact dialog config
    QDBusInterface kmailIface( QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QLatin1String("org.kde.kmail.kmail"),
                               QDBusConnection::sessionBus() );
    if ( kmailIface.isValid() ) {
        QDBusReply<void> reply;
        if ( !( reply = kmailIface.call( QLatin1String("updateConfig") ) ).isValid() ) {
            QDBusError err = kmailIface.lastError();
            kError() << "Communication problem with KMail. "
                     << "Error message was:" << err.name() << ": \"" << err.message() << "\"";
        }
    }
    QDBusInterface knotesIface( QLatin1String("org.kde.kontact"), QLatin1String("/KNotes"), QLatin1String("org.kde.kontact.KNotes"),
                                QDBusConnection::sessionBus() );
    if ( knotesIface.isValid() ) {
        QDBusReply<void> reply;
        if ( !( reply = knotesIface.call( QLatin1String("updateConfig") ) ).isValid() ) {
            const QDBusError err = knotesIface.lastError();
            kError() << "Communication problem with KNotes. "
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

