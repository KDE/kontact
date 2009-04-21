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
  : Kontact::Plugin( core, core, "kcontactmanager" )
{
  setComponentData( KontactPluginFactory::componentData() );

}

KContactManagerPlugin::~KContactManagerPlugin()
{
}

KParts::ReadOnlyPart *KContactManagerPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  return part;
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
#if 0
    org::kde::KAddressbook::Core kaddressbook(
      "org.kde.kaddressbook", "/KAddressBook", QDBusConnection::sessionBus() );
    QDBusReply<bool> reply = kaddressbook.handleCommandLine();
    if ( reply.isValid() ) {
        bool handled = reply;
        kDebug() << "handled=" << handled;
        if ( !handled ) { // no args -> simply bring kaddressbook plugin to front
          return Kontact::UniqueAppHandler::newInstance();
        }
    }
#endif
    return 0;
}
