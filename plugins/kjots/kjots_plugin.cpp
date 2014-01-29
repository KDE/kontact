/*
  This file is part of KJots.

  Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "kjots_plugin.h"

#include <KontactInterface/Core>

#include <KAction>
#include <KActionCollection>
#include <KCmdLineArgs>
#include <KIcon>
#include <KLocalizedString>

EXPORT_KONTACT_PLUGIN( KJotsPlugin, kjots )

KJotsPlugin::KJotsPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "kjots" ), m_interface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action =
    new KAction( KIcon( QLatin1String("document-new") ),
                 i18nc( "@action:inmenu", "New KJots Page" ), this );
  actionCollection()->addAction( QLatin1String("new_kjots_page"), action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_P ) );
  action->setHelpText(
    i18nc( "@info:status", "Create a new jots page" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can create a new jots page." ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(newPage()) );
  insertNewAction( action );

  action = new KAction( KIcon( QLatin1String("address-book-new") ),
                        i18nc( "@action:inmenu", "New KJots Book" ), this );
  actionCollection()->addAction( QLatin1String("new_kjots_book"), action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_B ) );
  action->setHelpText(
    i18nc( "@info:status", "Create a new jots book" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can create a new jots book." ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(newBook()) );
  insertNewAction( action );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<KJotsUniqueAppHandler>(), this );
}

KJotsPlugin::~KJotsPlugin()
{
  delete m_interface;
  m_interface = 0;
}

bool KJotsPlugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KJotsPlugin::invisibleToolbarActions() const
{
  return QStringList() << QLatin1String("new_page") << QLatin1String("new_book") ;
}

KParts::ReadOnlyPart *KJotsPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  m_interface = new OrgKdeKJotsWidgetInterface(
    QLatin1String("org.kde.kjots"), QLatin1String("/KJotsWidget"), QDBusConnection::sessionBus() );

  return part;
}

OrgKdeKJotsWidgetInterface *KJotsPlugin::interface()
{
  if ( !m_interface ) {
    part();
  }
  Q_ASSERT( m_interface );
  return m_interface;
}

void KJotsPlugin::newPage()
{
  core()->selectPlugin( this );
  interface()->newPage();
}

void KJotsPlugin::newBook()
{
  core()->selectPlugin( this );
  interface()->newBook();
}

bool KJotsPlugin::queryClose() const
{
  if ( m_interface ) {
    return m_interface->queryClose();
  }
  return true;
}

void KJotsUniqueAppHandler::loadCommandLineOptions()
{
  //  No command line args to load.
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );
}

int KJotsUniqueAppHandler::newInstance()
{
  // Ensure part is loaded
  (void)plugin()->part();
#if 0
  org::kde::KJotsWidget kjots(
    "org.kde.kjots", "/KJotsWidget", QDBusConnection::sessionBus() );
#endif
  return KontactInterface::UniqueAppHandler::newInstance();

}



#include "moc_kjots_plugin.cpp"
