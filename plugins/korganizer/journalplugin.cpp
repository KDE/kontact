/*
  This file is part of Kontact.

  Copyright (c) 2004 Allen Winter <winter@kde.org>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "journalplugin.h"
#include "korg_uniqueapp.h"
#include "calendarinterface.h"

#include <kontactinterfaces/core.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <QtDBus/QtDBus>

EXPORT_KONTACT_PLUGIN( JournalPlugin, journal )

JournalPlugin::JournalPlugin( Kontact::Core *core, const QVariantList & )
  : Kontact::Plugin( core, core, "korganizer" ), mIface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );
  KIconLoader::global()->addAppDir( "korganizer" );
  KIconLoader::global()->addAppDir( "kdepim" );

  KAction *action = new KAction( KIcon( "journal-new" ), i18n( "New Journal..." ), this );
  actionCollection()->addAction( "new_journal", action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_J ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotNewJournal()) );
  insertNewAction( action );

  KAction *syncAction = new KAction( KIcon( "view-refresh" ), i18n( "Sync Journal" ), this );
  actionCollection()->addAction( "journal_sync", syncAction );
  connect( syncAction, SIGNAL(triggered(bool)), SLOT(slotSyncJournal()) );
  insertSyncAction( syncAction );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
    new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

JournalPlugin::~JournalPlugin()
{
}

KParts::ReadOnlyPart *JournalPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part ) {
    return 0;
  }

  mIface = new OrgKdeKorganizerCalendarInterface(
    "org.kde.korganizer", "/Calendar", QDBusConnection::sessionBus(), this );

  return part;
}

void JournalPlugin::select()
{
  interface()->showJournalView();
}

QStringList JournalPlugin::invisibleToolbarActions() const
{
  QStringList invisible;
  invisible += "new_event";
  invisible += "new_todo";
  invisible += "new_journal";

  invisible += "view_whatsnext";
  invisible += "view_day";
  invisible += "view_nextx";
  invisible += "view_month";
  invisible += "view_workweek";
  invisible += "view_week";
  invisible += "view_list";
  invisible += "view_todo";
  invisible += "view_journal";
  invisible += "view_timeline";
  invisible += "view_timespent";

  return invisible;
}

OrgKdeKorganizerCalendarInterface *JournalPlugin::interface()
{
  if ( !mIface ) {
    part();
  }
  Q_ASSERT( mIface );
  return mIface;
}

void JournalPlugin::slotNewJournal()
{
  interface()->openJournalEditor( "", QDate() );
}

void JournalPlugin::slotSyncJournal()
{
  QDBusMessage message =
      QDBusMessage::createMethodCall( "org.kde.kmail", "/Groupware",
                                      "org.kde.kmail.groupware",
                                      "triggerSync" );
  message << QString( "Journal" );
  QDBusConnection::sessionBus().send( message );
}

bool JournalPlugin::createDBUSInterface( const QString &serviceType )
{
  if ( serviceType == "DBUS/Organizer" || serviceType == "DBUS/Calendar" ) {
    if ( part() ) {
      return true;
    }
  }
  return false;
}

bool JournalPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

#include "journalplugin.moc"
