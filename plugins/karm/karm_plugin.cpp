/*
    This file is part of Kontact.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    adapted for karm 2005 by Thorsten Staerk <kde@staerk.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "plugin.h"

#include "karm_plugin.h"
#include "karmdcopiface_stub.h"

typedef KGenericFactory<KarmPlugin, Kontact::Core> KarmPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_karm,
                            KarmPluginFactory( "kontact_karm" ) )

KarmPlugin::KarmPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "KArm" )
{
  setInstance( KarmPluginFactory::instance() );
  (void)dcopClient();
  insertNewAction( new KAction( i18n( "New Task" ), "karm",
                   CTRL+SHIFT+Key_W, this, SLOT( newTask() ), actionCollection(),
                   "new_task" ) );
}

KarmPlugin::~KarmPlugin()
{
}

KParts::ReadOnlyPart* KarmPlugin::createPart()
{
  KParts::ReadOnlyPart * part = loadPart();
  if ( !part ) return 0;

  // this calls a DCOP interface from karm via the lib KarmDCOPIface_stub that is generated automatically
  mStub = new KarmDCOPIface_stub( dcopClient(), "KArm",
                                      "KarmDCOPIface" );

  return part;
}

void KarmPlugin::newTask()
{
  kdDebug() << "Entering newTask" << endl;
  mStub->addTask("New Task");
}

#include "karm_plugin.moc"
