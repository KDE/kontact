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

#include <QWidget>
#include <QDropEvent>

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

#include <libkdepim/maillistdrag.h>
#include <kicon.h>
#include "core.h"
#include "plugin.h"
#include "coreinterface.h" 
#include "kmailinterface.h" 
#include "kaddressbook_plugin.h"

typedef KGenericFactory< KAddressbookPlugin, Kontact::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kaddressbookplugin,
                            KAddressbookPluginFactory( "kontact_kaddressbookplugin" ) )

KAddressbookPlugin::KAddressbookPlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "kaddressbook" ),
    m_interface( 0 )
{
  setInstance( KAddressbookPluginFactory::instance() );

  KAction *action = new KAction( KIcon("identity"), i18n( "New Contact..." ), actionCollection(), "new_contact" );
  connect(action, SIGNAL(triggered(bool)), SLOT( slotNewContact()));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_C));
  insertNewAction(action);
  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KABUniqueAppHandler>(), this );
}

KAddressbookPlugin::~KAddressbookPlugin()
{
}

QString KAddressbookPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "kaddressbook/tips");
  QString file;
  return file;
}

  KParts::ReadOnlyPart* KAddressbookPlugin::createPart()
{
  KParts::ReadOnlyPart * part = loadPart();
  if ( !part ) return 0;

  // Create the stub that allows us to talk to the part
  m_interface = new OrgKdeKAddressbookCoreInterface( "org.kde.kaddressbook", "/KAddressBook", QDBusConnection::sessionBus() );
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

OrgKdeKAddressbookCoreInterface *KAddressbookPlugin::interface()
{
  if ( !m_interface ) {
    part();
  }
  Q_ASSERT( m_interface );
  return m_interface;
}

void KAddressbookPlugin::slotNewContact()
{
  interface()->newContact();
}

bool KAddressbookPlugin::createDCOPInterface( const QString& serviceType )
{
  if ( serviceType == "DCOP/AddressBook" )  {
    Q_ASSERT( m_interface );
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

bool KAddressbookPlugin::canDecodeMimeData( const QMimeData * mimeData )
{
  return mimeData->hasText() || KPIM::MailList::canDecode( mimeData );
}

void KAddressbookPlugin::processDropEvent( QDropEvent *event )
{
  const QMimeData *md = event->mimeData();
  if ( KPIM::MailList::canDecode( md ) ) {
    event->accept();
    KPIM::MailList mails = KPIM::MailList::fromMimeData( md );
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n( "Drops of multiple mails are not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();

#warning Port me to DBus!
/*      KMailIface_stub kmailIface( "kmail", "KMailIface" );
      QString sFrom = kmailIface.getFrom( mail.serialNumber() );

      if ( !sFrom.isEmpty() ) {
        KAddrBookExternal::addEmail( sFrom, core() );
      }*/
    }
    return;
  }

  KMessageBox::sorry( core(), i18n( "Cannot handle drop events of type '%1'." ,
                        event->format() ) );
}

////

#include "../../../kaddressbook/kaddressbook_options.h"

void KABUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( kaddressbook_options );
}

int KABUniqueAppHandler::newInstance()
{
    kDebug(5602) << k_funcinfo << endl;
    // Ensure part is loaded
    (void)plugin()->part();

    org::kde::KAddressbook::Core kaddressbook("org.kde.kaddressbook", "/KAddressBook", QDBusConnection::sessionBus());
    QDBusReply<bool> reply = kaddressbook.handleCommandLine();
    if(reply.isValid() ) {
        bool handled = reply;
        kDebug(5602) << k_funcinfo << "handled=" << handled << endl;
        if ( !handled ) // no args -> simply bring kaddressbook plugin to front
            return Kontact::UniqueAppHandler::newInstance();    
    }

    return 0;
}

#include "kaddressbook_plugin.moc"

// vim: sw=2 sts=2 tw=80 et
