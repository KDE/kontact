/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#include <qwidget.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <kontact/core.h>
#include <kontact/plugin.h>

#include "akregator_plugin.h"

typedef KGenericFactory< aKregatorPlugin, Kontact::Core > aKregatorPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_akregator,
                            aKregatorPluginFactory( "kontact_akregator" ) )

aKregatorPlugin::aKregatorPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "akregator" )
{
  setInstance( aKregatorPluginFactory::instance() );
}

aKregatorPlugin::~aKregatorPlugin()
{
}

KParts::Part* aKregatorPlugin::createPart()
{
  return loadPart();
}

QStringList aKregatorPlugin::configModules() const
{
  QStringList modules;
  modules << "PIM/akregator.desktop";
  return modules;
}

#include "akregator_plugin.moc"
