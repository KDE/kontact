/*
  This file is part of Akregator.

  Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

#include "akregator_plugin.h"
#include "partinterface.h"

#include <akregator_options.h>
#include <akregator_part.h>

#include <kontactinterfaces/core.h>
#include <kontactinterfaces/plugin.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

EXPORT_KONTACT_PLUGIN( AkregatorPlugin, akregator )

AkregatorPlugin::AkregatorPlugin( Kontact::Core *core, const QVariantList & )
  : Kontact::Plugin( core, core, "akregator" ), m_interface( 0 )
{

  setComponentData( KontactPluginFactory::componentData() );

  KAction *action  = new KAction( KIcon( "bookmark-new" ), i18n( "New Feed..." ), this );
  actionCollection()->addAction( "feed_new", action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_F ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(addFeed()) );
  insertNewAction( action );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
    new Kontact::UniqueAppHandlerFactory<AkregatorUniqueAppHandler>(), this );
}

AkregatorPlugin::~AkregatorPlugin()
{
}

bool AkregatorPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList AkregatorPlugin::invisibleToolbarActions() const
{
  return QStringList( "file_new_contact" );
}

OrgKdeAkregatorPartInterface *AkregatorPlugin::interface()
{
  if ( !m_interface ) {
    part();
  }
  Q_ASSERT( m_interface );
  return m_interface;

}

QString AkregatorPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "akregator/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart *AkregatorPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  connect( part, SIGNAL(showPart()), this, SLOT(showPart()) );
  m_interface = new OrgKdeAkregatorPartInterface(
    "org.kde.akregator", "/Akregator", QDBusConnection::sessionBus() );
  m_interface->openStandardFeedList();

  return part;
}

void AkregatorPlugin::showPart()
{
  core()->selectPlugin( this );
}

void AkregatorPlugin::addFeed()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_interface );
  m_interface->addFeed();
}

QStringList AkregatorPlugin::configModules() const
{
  QStringList modules;
  modules << "PIM/akregator.desktop";
  return modules;
}

void AkregatorPlugin::readProperties( const KConfigGroup &config )
{
  if ( part() ) {
    Akregator::Part *myPart = static_cast<Akregator::Part*>( part() );
    myPart->readProperties( config );
  }
}

void AkregatorPlugin::saveProperties( KConfigGroup &config )
{
  if ( part() ) {
    Akregator::Part *myPart = static_cast<Akregator::Part*>( part() );
    myPart->saveProperties( config );
  }
}

#include "../../../akregator/src/akregator_options.h"
void AkregatorUniqueAppHandler::loadCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( Akregator::akregator_options() );
}

int AkregatorUniqueAppHandler::newInstance()
{
  kDebug();
  // Ensure part is loaded
  (void)plugin()->part();

  org::kde::akregator::part akregator(
    "org.kde.akregator", "/Akregator", QDBusConnection::sessionBus() );
  akregator.openStandardFeedList();

  return Kontact::UniqueAppHandler::newInstance();
}

#include "akregator_plugin.moc"
