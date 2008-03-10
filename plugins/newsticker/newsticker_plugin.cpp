/*
   This file is part of Kontact.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <kgenericfactory.h>
#include <klocale.h>
#include <kparts/componentfactory.h>
#include "core.h"

#include "summarywidget.h"

#include "newsticker_plugin.h"

EXPORT_KONTACT_PLUGIN(NewsTickerPlugin, newsticker)

NewsTickerPlugin::NewsTickerPlugin( Kontact::Core *core, const QVariantList& )
  : Kontact::Plugin( core, core, 0 )
{
	setComponentData( KontactPluginFactory::componentData() );
}

Kontact::Summary *NewsTickerPlugin::createSummaryWidget( QWidget* parentWidget )
{
  return new SummaryWidget( parentWidget );
}
