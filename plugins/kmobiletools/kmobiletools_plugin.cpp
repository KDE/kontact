/***************************************************************************
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (C) 2007 Marco Gulino <marco@kmobiletools.org>
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "kmobiletools_plugin.h"
#include <core.h>
#include <maininterface.h>

#include <kparts/componentfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <kgenericfactory.h>

EXPORT_KONTACT_PLUGIN(KMobileToolsPlugin, kmobiletools)

KMobileToolsPlugin::KMobileToolsPlugin( Kontact::Core *core, const QVariantList & )
  : Kontact::Plugin( core, core, "KMobileTools" ), partLoaded( false )
{
  setComponentData( KontactPluginFactory::componentData() );
  KAction *newaction = new KAction( KIcon( "newsms" ), i18n( "New SMS..." ), this );
  actionCollection()->addAction( "newsms", newaction );
  newaction->setShortcut( QKeySequence( Qt::CTRL+Qt::SHIFT + Qt::Key_S ) );
  connect( newaction, SIGNAL(triggered(bool)),SLOT(slotNewSMS()) );
  insertNewAction( newaction );
  setExecutableName( "kmobiletools_bin" );
}

KMobileToolsPlugin::~KMobileToolsPlugin()
{
}

bool KMobileToolsPlugin::isRunningStandalone()
{
//     return ( (!partLoaded) && kapp->dcopClient()->isApplicationRegistered("kmobiletools") );
#ifdef __GNUC__
#warning FIXME!!!
#endif
  return false;
}

KParts::ReadOnlyPart *KMobileToolsPlugin::createPart()
{
  KParts::ReadOnlyPart *m_part = loadPart();
  partLoaded=(bool)m_part;
#ifdef __GNUC__
#warning "look at if interface is ok";
#endif
  m_interface = new OrgKdeKmobiletoolsMainInterface( "org.kde.kmobiletools",
                                                     "/KMobitools", QDBusConnection::sessionBus() );
  return m_part;
}

void KMobileToolsPlugin::slotNewSMS()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_interface );
  m_interface->newSMS();
}

#include "kmobiletools_plugin.moc"
