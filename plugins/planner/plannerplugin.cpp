/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004-2005,2009 Allen Winter <winter@kde.org>

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

#include "plannerplugin.h"
#include "planner.h"

#include <KontactInterface/Core>

#include <KAboutData>
#include <KLocalizedString>

EXPORT_KONTACT_PLUGIN( PlannerPlugin, planner )

PlannerPlugin::PlannerPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, 0 ),
    mAboutData( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );
}

PlannerPlugin::~PlannerPlugin()
{
}

KontactInterface::Summary *PlannerPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new Planner( this, parentWidget );
}

const KAboutData *PlannerPlugin::aboutData() const
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "planner", 0,
                                 ki18n( "Planner Summary" ),
                                 "1.0",
                                 ki18n( "Kontact Planner Summary" ),
                                 KAboutData::License_LGPL,
                                 ki18n( "Copyright © 2003 Tobias Koenig\n"
                                        "Copyright © 2004–2010 Allen Winter" ) );
    mAboutData->addAuthor( ki18n( "Allen Winter" ),
                           ki18n( "Current Maintainer" ), "winter@kde.org" );
    mAboutData->addAuthor( ki18n( "Tobias Koenig" ),
                           KLocalizedString(), "tokoe@kde.org" );
    mAboutData->addAuthor( ki18n( "Oral Timocin" ),
                           KLocalizedString(), "oral.timocin@kdemail.net" );
    mAboutData->setProductName( "kontact/planner" );
  }

  return mAboutData;
}
