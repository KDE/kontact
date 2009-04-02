/*
    This file is part of Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>
#include <qdragobject.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <kaddrbook.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <dcopclient.h>
#include "kmailIface_stub.h"

#include <libkdepim/maillistdrag.h>

#include "core.h"
#include "plugin.h"

#include "kaddressbook_plugin.h"

typedef KGenericFactory< KAddressbookPlugin, Kontact::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kaddressbookplugin,
                            KAddressbookPluginFactory( "kontact_kaddressbookplugin" ) )

KAddressbookPlugin::KAddressbookPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "kaddressbook" ),
    mStub( 0 )
{
  setInstance( KAddressbookPluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Contact..." ), "identity",
			             CTRL+SHIFT+Key_C, this, SLOT( slotNewContact() ), actionCollection(),
                   "new_contact" ) );

  insertNewAction( new KAction( i18n( "&New Distribution List..." ), "kontact_contacts", 0, this,
                                SLOT( slotNewDistributionList() ), actionCollection(), "new_distributionlist" ) );

  insertSyncAction( new KAction( i18n( "Synchronize Contacts" ), "reload",
                    0, this, SLOT( slotSyncContacts() ), actionCollection(),
                   "kaddressbook_sync" ) );
  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KABUniqueAppHandler>(), this );
}

KAddressbookPlugin::~KAddressbookPlugin()
{
}

KParts::ReadOnlyPart* KAddressbookPlugin::createPart()
{
  KParts::ReadOnlyPart * part = loadPart();
  if ( !part ) return 0;

  // Create the stub that allows us to talk to the part
  mStub = new KAddressBookIface_stub( dcopClient(), "kaddressbook",
                                      "KAddressBookIface" );
  return part;
}

QStringList KAddressbookPlugin::configModules() const
{
  QStringList modules;
  modules << "PIM/kabconfig.desktop" << "PIM/kabldapconfig.desktop";
  return modules;
}

QStringList KAddressbookPlugin::invisibleToolbarActions() const
{
  return QStringList( "file_new_contact" );
}

KAddressBookIface_stub *KAddressbookPlugin::interface()
{
  if ( !mStub ) {
    part();
  }
  Q_ASSERT( mStub );
  return mStub;
}

void KAddressbookPlugin::slotNewContact()
{
  interface()->newContact();
}


void KAddressbookPlugin::slotNewDistributionList()
{
  interface()->newDistributionList();
}

void KAddressbookPlugin::slotSyncContacts()
{
  DCOPRef ref( "kmail", "KMailICalIface" );
  ref.send( "triggerSync", QString("Contact") );
}

bool KAddressbookPlugin::createDCOPInterface( const QString& serviceType )
{
  if ( serviceType == "DCOP/AddressBook" )  {
    Q_ASSERT( mStub );
    return true;
  }

  return false;
}

void KAddressbookPlugin::configUpdated()
{
}

bool KAddressbookPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

bool KAddressbookPlugin::canDecodeDrag( QMimeSource *mimeSource )
{
  return KPIM::MailListDrag::canDecode( mimeSource );
}

#include <dcopref.h>

void KAddressbookPlugin::processDropEvent( QDropEvent *event )
{
  KPIM::MailList mails;
  if ( KPIM::MailListDrag::decode( event, mails ) ) {
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n( "Drops of multiple mails are not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();

      KMailIface_stub kmailIface( "kmail", "KMailIface" );
      QString sFrom = kmailIface.getFrom( mail.serialNumber() );

      if ( !sFrom.isEmpty() ) {
        KAddrBookExternal::addEmail( sFrom, core() );
      }
    }
    return;
  }

  KMessageBox::sorry( core(), i18n( "Cannot handle drop events of type '%1'." )
                      .arg( event->format() ) );
}


void KAddressbookPlugin::loadProfile( const QString& directory )
{
  DCOPRef ref( "kaddressbook", "KAddressBookIface" );
  ref.send( "loadProfile", directory );
}

void KAddressbookPlugin::saveToProfile( const QString& directory ) const
{
  DCOPRef ref( "kaddressbook", "KAddressBookIface" );
  ref.send( "saveToProfile", directory );
}

////

#include "../../../kaddressbook/kaddressbook_options.h"

void KABUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( kaddressbook_options );
}

int KABUniqueAppHandler::newInstance()
{
    kdDebug(5602) << k_funcinfo << endl;
    // Ensure part is loaded
    (void)plugin()->part();
    DCOPRef kAB( "kaddressbook", "KAddressBookIface" );
    DCOPReply reply = kAB.call( "handleCommandLine" );
    if ( reply.isValid() ) {
        bool handled = reply;
        kdDebug(5602) << k_funcinfo << "handled=" << handled << endl;
        if ( !handled ) // no args -> simply bring kaddressbook plugin to front
            return Kontact::UniqueAppHandler::newInstance();
    }
    return 0;
}

#include "kaddressbook_plugin.moc"

// vim: sw=2 sts=2 tw=80 et
