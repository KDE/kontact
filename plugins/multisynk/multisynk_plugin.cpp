/*
    This file is part of Kontact.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "plugin.h"

#include "multisynk_plugin.h"

typedef KGenericFactory<MultiSynkPlugin, Kontact::Core> MultiSynkPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_multisynk,
                            MultiSynkPluginFactory( "kontact_multisynk" ) )

MultiSynkPlugin::MultiSynkPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "MultiSynk" )
{
  setInstance( MultiSynkPluginFactory::instance() );
}

MultiSynkPlugin::~MultiSynkPlugin()
{
}

KParts::ReadOnlyPart* MultiSynkPlugin::createPart()
{
  return loadPart();
}

#include "multisynk_plugin.moc"
