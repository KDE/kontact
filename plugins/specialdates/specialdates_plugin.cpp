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

#include "specialdates_plugin.h"
#include "sdsummarywidget.h"

#include <KontactInterface/Core>

#include <KAboutData>
#include <KLocalizedString>
#include <KIconLoader>

//QT5 EXPORT_KONTACT_PLUGIN( SpecialdatesPlugin, specialdates )

SpecialdatesPlugin::SpecialdatesPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, 0 ), mAboutData( 0 )
{
  //QT5 setComponentData( KontactPluginFactory::componentData() );
  KIconLoader::global()->addAppDir( QLatin1String("kdepim") );
}

SpecialdatesPlugin::~SpecialdatesPlugin()
{
}

KontactInterface::Summary *SpecialdatesPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new SDSummaryWidget( this, parentWidget );
}
#if 0
const KAboutData *SpecialdatesPlugin::aboutData() const
{
#if 0 //QT5
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "specialdates", 0,
                                 ki18n( "Special Dates Summary" ),
                                 "1.0",
                                 ki18n( "Kontact Special Dates Summary" ),
                                 KAboutData::License_LGPL,
                                 ki18n( "Copyright © 2003 Tobias Koenig\n"
                                        "Copyright © 2004–2010 Allen Winter" ) );
    mAboutData->addAuthor( ki18n( "Allen Winter" ),
                           ki18n( "Current Maintainer" ), "winter@kde.org" );
    mAboutData->addAuthor( ki18n( "Tobias Koenig" ),
                           KLocalizedString(), "tokoe@kde.org" );
    mAboutData->setProductName( "kontact/specialdates" );
  }
  return mAboutData;
#else
  return new KAboutData();
#endif
}
#endif
