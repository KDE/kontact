/*
  This file is part of Kontact.
  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004-2005 Allen Winter <winter@kde.org>

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

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "planner.h"

#include "plannerplugin.h"

typedef KGenericFactory< PlannerPlugin, Kontact::Core > PlannerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_plannerplugin,
                            PlannerPluginFactory( "kontact_plannerplugin" ) )

  PlannerPlugin::PlannerPlugin( Kontact::Core *core,
                                const char *name, const QStringList & )
    : Kontact::Plugin( core, core, name ),
      mAboutData( 0 )
{
  setInstance( PlannerPluginFactory::instance() );
}

PlannerPlugin::~PlannerPlugin()
{
}

Kontact::Summary *PlannerPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new Planner( this, parentWidget );
}

const KAboutData *PlannerPlugin::aboutData()
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "planner",
                                 I18N_NOOP( "Planner Summary" ),
                                 "1.0",
                                 I18N_NOOP( "Kontact Planner Summary" ),
                                 KAboutData::License_LGPL,
                                 I18N_NOOP( "(c) 2004-2005 The KDE PIM Team" ) );
    mAboutData->addAuthor( "Allen Winter", "Current Maintainer", "winter@kde.org" );
    mAboutData->addAuthor( "Tobias Koenig", "", "tokoe@kde.org" );
    mAboutData->addAuthor( "Oral Timocin", "", "oral.timocin@kdemail.net" );
    mAboutData->setProductName( "kontact/planner" );
  }

  return mAboutData;
}
