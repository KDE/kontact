/*
    This file is part of KAddressBook Kontact Plugin.

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddressbook_plugin.h"

#include <kontactinterface/core.h>

#include <kactioncollection.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

EXPORT_KONTACT_PLUGIN( KAddressBookPlugin, kaddressbook )

KAddressBookPlugin::KAddressBookPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "kaddressbook" )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action  = new KAction( KIcon( "contact-new" ),
                                  i18n( "New Contact..." ), this );
  actionCollection()->addAction( "new_contact", action );
  connect( action, SIGNAL( triggered( bool) ), SLOT( slotNewContact() ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
  insertNewAction( action );

  action  = new KAction( KIcon( "user-group-new" ),
                         i18n( "New Contact Group..." ), this );
  actionCollection()->addAction( "new_contactgroup", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotNewContactGroup() ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_G ) );
  action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );
  insertNewAction( action );

  KAction *syncAction = new KAction( KIcon( "view-refresh" ),
                                     i18n( "Synchronize Contacts" ), this );
  actionCollection()->addAction( "kaddressbook_sync", syncAction );
  connect( syncAction, SIGNAL(triggered(bool)), SLOT(slotSyncContacts()) );
  insertSyncAction( syncAction );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<KAddressBookUniqueAppHandler>(), this );
}

KAddressBookPlugin::~KAddressBookPlugin()
{
}

void KAddressBookPlugin::slotNewContact()
{
  KParts::ReadOnlyPart *part = createPart();
  if ( !part )
    return;

  if ( part->metaObject()->indexOfMethod( "newContact()" ) == -1 ) {
    kWarning() << "KAddressBook part is missing slot newContact()";
    return;
  }

  QMetaObject::invokeMethod( part, "newContact" );
}

void KAddressBookPlugin::slotNewContactGroup()
{
  KParts::ReadOnlyPart *part = createPart();
  if ( !part )
    return;

  if ( part->metaObject()->indexOfMethod( "newGroup()" ) == -1 ) {
    kWarning() << "KAddressBook part is missing slot newGroup()";
    return;
  }

  QMetaObject::invokeMethod( part, "newGroup" );
}

QString KAddressBookPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "kaddressbook/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart *KAddressBookPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  // disable the Ctrl+N shortcut, as it is used by Kontact already
  if ( part->action( "akonadi_contact_create" ) ) {
    KAction *newAction = qobject_cast<KAction*>( part->action( "akonadi_contact_create" ) );
    if ( newAction )
      newAction->setShortcut( QKeySequence() );
  }

  return part;
}

bool KAddressBookPlugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

//QStringList KAddressBookPlugin::invisibleToolbarActions() const
//{
  //QStringList actions;
  //return actions;
//}

void KAddressBookPlugin::slotSyncContacts()
{
  QDBusMessage message =
      QDBusMessage::createMethodCall( "org.kde.kmail", "/Groupware",
                                      "org.kde.kmail.groupware",
                                      "triggerSync" );
  message << QString( "Contact" );
  QDBusConnection::sessionBus().send( message );
}

void KAddressBookUniqueAppHandler::loadCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );
}

int KAddressBookUniqueAppHandler::newInstance()
{
    kDebug() ;
    // Ensure part is loaded
    (void)plugin()->part();
    return KontactInterface::UniqueAppHandler::newInstance();
}

