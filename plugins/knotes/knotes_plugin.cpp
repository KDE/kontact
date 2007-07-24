/*
       This file is part of Kontact
       Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

       This library is free software; you can redistribute it and/or
       modify it under the terms of the GNU Library General Public
       License as published by the Free Software Foundation; either
       version 2 of the License, or (at your option) any later version.

       This library is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
       Library General Public License for more details.

       You should have received a copy of the GNU Library General Public License
       along with this library; see the file COPYING.LIB.  If not, write to
       the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
       Boston, MA 02110-1301, USA.
*/

#include <QtDBus/QtDBus>

#include <kaboutdata.h>
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <kicon.h>
#include "core.h"
#include "knotes_part.h"
#include "summarywidget.h"

#include "knotes_plugin.h"

typedef KGenericFactory< KNotesPlugin, Kontact::Core > KNotesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_knotesplugin,
                            KNotesPluginFactory( "kontact_knotesplugin" ) )


KNotesPlugin::KNotesPlugin( Kontact::Core *core, const QStringList & )
  : Kontact::Plugin( core, core, "knotes" ),
    mAboutData( 0 )
{
  setComponentData( KNotesPluginFactory::componentData() );

    KAction *action  = new KAction(KIcon("knotes"), i18n("New Note..."), this);
    actionCollection()->addAction("new_note", action );
  connect(action, SIGNAL(triggered(bool)), SLOT( slotNewNote() ));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_N));
  insertNewAction(action);

  KAction* syncAction = new KAction(KIcon("reload"), i18n( "Synchronize Notes" ), this);
  actionCollection()->addAction("knotes_sync", syncAction);
  connect(action, SIGNAL(triggered(bool)), SLOT( slotSyncNotes()));
  insertSyncAction( syncAction );

}

KNotesPlugin::~KNotesPlugin()
{
}

QString KNotesPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "knotes/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart* KNotesPlugin::createPart()
{
  return new KNotesPart( this /*, "notes"*/ );
}

Kontact::Summary *KNotesPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new KNotesSummaryWidget( this, parentWidget );
}

const KAboutData *KNotesPlugin::aboutData()
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "knotes", 0, ki18n( "Notes Management" ),
                                 "0.5", ki18n( "Notes Management" ),
                                  KAboutData::License_GPL_V2,
                                  ki18n("(c) 2003-2004 The Kontact developers") );
    mAboutData->addAuthor( ki18n("Michael Brade"), ki18n("Current Maintainer"), "brade@kde.org" );
    mAboutData->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );
  }

  return mAboutData;
}


// private slots

void KNotesPlugin::slotNewNote()
{
  if ( part() )
      static_cast<KNotesPart *>( part() )->newNote();
}

void KNotesPlugin::slotSyncNotes()
{
  QDBusMessage message = QDBusMessage::createSignal( "/KMailICalIface", "org.kde.kmail", "triggerSync(QString)");
  message << QString( "Note" );
  QDBusConnection::sessionBus().send( message );

}

#include "knotes_plugin.moc"

