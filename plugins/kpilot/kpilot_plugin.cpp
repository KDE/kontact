/*
   This file is part of Kontact.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>
	 Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "options.h"

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "summarywidget.h"

#include "kpilot_plugin.h"

typedef KGenericFactory< KPilotPlugin, Kontact::Core > KPilotPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kpilotplugin,
                            KPilotPluginFactory( "kontact_kpilotplugin" ) )

KPilotPlugin::KPilotPlugin( Kontact::Core *core, const char *name, const QStringList& )
  : Kontact::Plugin( core, core, name ), mAboutData( 0 )
{
  setInstance( KPilotPluginFactory::instance() );
	// TODO: Make sure kpilotDaemon is running!


}

Kontact::Summary *KPilotPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new SummaryWidget( parentWidget );
}

const KAboutData *KPilotPlugin::aboutData()
{
  if ( !mAboutData ) {
	mAboutData = new KAboutData("kpilotplugin", I18N_NOOP("KPilot Information"),
		KPILOT_VERSION,
		I18N_NOOP("KPilot - HotSync software for KDE\n\n"),
		KAboutData::License_GPL, "(c) 2004 Reinhold Kainhofer");
	mAboutData->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Plugin Developer"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");
	mAboutData->addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		0, "http://www.kpilot.org/");
	mAboutData->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://people.fruitsalad.org/adridg/");
  }

  return mAboutData;
}
