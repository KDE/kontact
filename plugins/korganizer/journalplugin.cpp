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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>

#include <kapplication.h>
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <dcopclient.h>

#include "core.h"

#include "journalplugin.h"
#include "korg_uniqueapp.h"

typedef KGenericFactory< JournalPlugin, Kontact::Core > JournalPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_journalplugin,
                            JournalPluginFactory( "kontact_journalplugin" ) )

JournalPlugin::JournalPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{
  setInstance( JournalPluginFactory::instance() );
  instance()->iconLoader()->addAppDir("korganizer");

  insertNewAction( new KAction( i18n( "New Journal..." ), "newjournal",
                   CTRL+SHIFT+Key_J, this, SLOT( slotNewJournal() ), actionCollection(),
                   "new_journal" ) );


  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

JournalPlugin::~JournalPlugin()
{
}

KParts::ReadOnlyPart *JournalPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part )
    return 0;

  dcopClient(); // ensure that we register to DCOP as "korganizer"
  mIface = new KCalendarIface_stub( dcopClient(), "kontact", "CalendarIface" );

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

  invisible += "view_day";
  invisible += "view_list";
  invisible += "view_workweek";
  invisible += "view_week";
  invisible += "view_nextx";
  invisible += "view_month";
  invisible += "view_todo";
  return invisible;
}

KCalendarIface_stub *JournalPlugin::interface()
{
  if ( !mIface ) {
    part();
  }
  Q_ASSERT( mIface );
  return mIface;
}

void JournalPlugin::slotNewJournal()
{
  interface()->openJournalEditor( "" );
}

bool JournalPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool JournalPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

#include "journalplugin.moc"
