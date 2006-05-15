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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "summarywidget.h"

#include "weather_plugin.h"

typedef KGenericFactory< WeatherPlugin, Kontact::Core > WeatherPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_weatherplugin,
                            WeatherPluginFactory( "kontact_weatherplugin" ) )

WeatherPlugin::WeatherPlugin( Kontact::Core *core, const char *name, const QStringList& )
  : Kontact::Plugin( core, core, name ), mAboutData( 0 )
{
  setInstance( WeatherPluginFactory::instance() );
}

Kontact::Summary *WeatherPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new SummaryWidget( parentWidget );
}

const KAboutData *WeatherPlugin::aboutData()
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "weatherplugin", I18N_NOOP( "Weather Information" ),
                                 "0.1",
                                 I18N_NOOP( "Weather Information" ),
                                 KAboutData::License_GPL_V2,
                                 "(c) 2003 The Kontact developers" );
    mAboutData->addAuthor( "Ian Reinhart Geiser", "", "geiseri@kde.org" );
    mAboutData->addAuthor( "Tobias Koenig", "", "tokoe@kde.org" );
    mAboutData->addCredit( "John Ratke",
                           I18N_NOOP( "Improvements and more code cleanups" ),
                           "jratke@comcast.net" );
  }

  return mAboutData;
}
