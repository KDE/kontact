/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  adapted for karm 2005 by Thorsten Staerk <kde@staerk.de>
  karm renamed to ktimetracker 2007-2008

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

#include "ktimetracker_plugin.h"
#include "ktimetrackerinterface.h"

#include <KontactInterface/Core>

#include <KActionCollection>
#include <KCmdLineArgs>
#include <KDebug>
#include <KIcon>
#include <KXMLGUIFactory>

#include <QMenu>

EXPORT_KONTACT_PLUGIN( ktimetrackerplugin, ktimetracker )

ktimetrackerplugin::ktimetrackerplugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "ktimetracker" ), mInterface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action = new KAction( KIcon( "ktimetracker" ),
                                 i18nc( "@action:inmenu", "New Task" ), this );
  actionCollection()->addAction( "new_task", action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_W ) );
  action->setHelpText( i18nc( "@info:status", "Create a new time tracker" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can create "
           "and start a new time tracker." ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(newTask()) );
  insertNewAction( action );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<KtimetrackerUniqueAppHandler>(), this );
}

ktimetrackerplugin::~ktimetrackerplugin()
{
  delete mInterface;
}

bool ktimetrackerplugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList ktimetrackerplugin::invisibleToolbarActions() const
{
  return QStringList() << "new_task" << "new_sub_task" ;
}

KParts::ReadOnlyPart *ktimetrackerplugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }
  mInterface = new OrgKdeKtimetrackerKtimetrackerInterface(
    "org.kde.ktimetracker", "/KTimeTracker", QDBusConnection::sessionBus() );
  // Setup context menu request handling
  connect( part->widget(),
           SIGNAL(contextMenuRequested(QPoint)),
           this,
           SLOT(taskViewCustomContextMenuRequested(QPoint)) );

  return part;
}

void ktimetrackerplugin::taskViewCustomContextMenuRequested( const QPoint &point )
{
  kDebug() << "entering function";
  QMenu *pop = dynamic_cast<QMenu*>( factory()->container( i18n( "task_popup" ), part() ) );
  if ( pop ) {
    pop->popup( point );
  }
}

OrgKdeKtimetrackerKtimetrackerInterface *ktimetrackerplugin::interface()
{
  if ( !mInterface ) {
    part();
  }
  Q_ASSERT( mInterface );
  return mInterface;
}

QStringList ktimetrackerplugin::configModules() const
{
  QStringList modules;
  modules << "PIM/ktimetrackerconfig.desktop";
  return modules;
}

void ktimetrackerplugin::newTask()
{
  kDebug() << "entering function";
  core()->selectPlugin( this );
  interface()->newTask();
}

void KtimetrackerUniqueAppHandler::loadCommandLineOptions()
{
  // TODO: handle command line options
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );
}

int KtimetrackerUniqueAppHandler::newInstance()
{
  // Ensure part is loaded
  (void)plugin()->part();
  return KontactInterface::UniqueAppHandler::newInstance();
}

