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
  QDBusInterface iface( "org.kde.kmail", "/KMail",
                        "org.kde.kmail.kmail",
                        QDBusConnection::sessionBus());
  if ( !iface.isValid() )
    return;
  
  QDBusReply<void> reply;
  if ( !( reply = iface.call( "updateConfig" ) ).isValid() )
  {
    QDBusError err = iface.lastError();
    kError() << "Communication problem with KMail. "
             << "Error message was:" << err.name() << ": \"" << err.message() << "\"";
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

#include "kontactconfiguredialog.moc"
