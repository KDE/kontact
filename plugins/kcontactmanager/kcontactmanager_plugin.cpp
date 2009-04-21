/*
    This file is part of KContactManager.

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

#include "kcontactmanager_plugin.h"
#include "mainwidgetinterface.h"

#include <kontactinterfaces/core.h>

#include <kactioncollection.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>


EXPORT_KONTACT_PLUGIN( KContactManagerPlugin, kcontactmanager )

KContactManagerPlugin::KContactManagerPlugin( Kontact::Core *core, const QVariantList & )
  : Kontact::Plugin( core, core, "kcontactmanager" ), m_interface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action  = new KAction( KIcon( "contact-new" ),
                                  i18n( "New Contact..." ), this );
  actionCollection()->addAction( "new_contact", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotNewContact()) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
  insertNewAction( action );

  KAction *syncAction = new KAction( KIcon( "view-refresh" ),
                                     i18n( "Synchronize Contacts" ), this );
  actionCollection()->addAction( "kaddressbook_sync", syncAction );
  connect( syncAction, SIGNAL(triggered(bool)), SLOT(slotSyncContacts()) );
  insertSyncAction( syncAction );


  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
    new Kontact::UniqueAppHandlerFactory<KContactManagerUniqueAppHandler>(), this );

}

KContactManagerPlugin::~KContactManagerPlugin()
{
}

void KContactManagerPlugin::slotNewContact()
{
  interface()->newContact();
}

QString KContactManagerPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "kcontactmanager/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart *KContactManagerPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }
  // Create the stub that allows us to talk to the part
  m_interface = new OrgKdeKContactmanagerMainWidgetInterface(
    "org.kde.kcontactmanager", "/KContactManager", QDBusConnection::sessionBus() );
//     org.kde.KContactmanager.MainWidget                                      /KContactManager
  return part;
}

OrgKdeKContactmanagerMainWidgetInterface *KContactManagerPlugin::interface()
{
  if ( !m_interface ) {
    part();
  }
  Q_ASSERT( m_interface );
  return m_interface;
}


bool KContactManagerPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}


QStringList KContactManagerPlugin::invisibleToolbarActions() const
{
  return QStringList( "file_new_contact" );
}

void KContactManagerPlugin::slotSyncContacts()
{
  QDBusMessage message =
      QDBusMessage::createMethodCall( "org.kde.kmail", "/Groupware",
                                      "org.kde.kmail.groupware",
                                      "triggerSync" );
  message << QString( "Contact" );
  QDBusConnection::sessionBus().send( message );
}

void KContactManagerUniqueAppHandler::loadCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );
}

int KContactManagerUniqueAppHandler::newInstance()
{
    kDebug() ;
    // Ensure part is loaded
    (void)plugin()->part();
    return Kontact::UniqueAppHandler::newInstance();
}

