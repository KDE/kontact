/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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

#include <qobjectlist.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kinstance.h>
#include <krun.h>

#include "core.h"
#include "plugin.h"

using namespace Kontact;

class Plugin::Private
{
  public:
    Kontact::Core *core;
    DCOPClient *dcopClient;
    QPtrList<KAction> *newActions;
    QString identifier;
    QString title;
    QString icon;
    QString executableName;
    QCString partLibraryName;
    bool hasPart;
    KParts::ReadOnlyPart *part;
};


Plugin::Plugin( Kontact::Core *core, QObject *parent, const char *name )
  : QObject( parent, name ), d( new Private )
{
  KGlobal::locale()->insertCatalogue(name);

  d->core = core;
  d->dcopClient = 0;
  d->newActions = new QPtrList<KAction>;
  d->hasPart = true;
  d->part = 0;
}


Plugin::~Plugin()
{
  delete d->part;
  delete d->dcopClient;
  delete d;
}

void Plugin::setIdentifier( const QString &identifier )
{
  d->identifier = identifier;
}

QString Plugin::identifier() const
{
  return d->identifier;
}

void Plugin::setTitle( const QString &title )
{
  d->title = title;
}

QString Plugin::title() const
{
  return d->title;
}

void Plugin::setIcon( const QString &icon )
{
  d->icon = icon;
}

QString Plugin::icon() const
{
  return d->icon;
}

void Plugin::setExecutableName( const QString& bin )
{
  d->executableName = bin;
}

QString Plugin::executableName() const
{
  return d->executableName;
}

void Plugin::setPartLibraryName( const QCString &libName )
{
  d->partLibraryName = libName;
}

KParts::ReadOnlyPart *Plugin::loadPart()
{
  return core()->createPart( d->partLibraryName );
}

const KAboutData *Plugin::aboutData()
{
  kdDebug(5601) << "Plugin::aboutData(): libname: " << d->partLibraryName << endl;

  const KInstance *instance =
    KParts::Factory::partInstanceFromLibrary( d->partLibraryName );

  if ( instance ) {
    return instance->aboutData();
  } else {
    kdError() << "Plugin::aboutData(): Can't load instance for "
              << title() << endl;
    return 0;
  }
}

KParts::ReadOnlyPart *Plugin::part()
{
  if ( !d->part ) {
    d->part = createPart();
    if ( d->part ) {
      connect( d->part, SIGNAL( destroyed() ), SLOT( partDestroyed() ) );
      core()->partLoaded( this, d->part );
    }
  }
  return d->part;
}

QString Plugin::tipFile() const
{
  return QString::null;
}


DCOPClient* Plugin::dcopClient() const
{
  if ( !d->dcopClient ) {
    d->dcopClient = new DCOPClient();
    // ### Note: maybe we could use executableName().latin1() instead here.
    // But this requires that dcopClient is NOT called by the constructor,
    // and is called by some new virtual void init() later on.
    d->dcopClient->registerAs( name(), false );
  }

  return d->dcopClient;
}

void Plugin::insertNewAction( KAction *action )
{
  d->newActions->append( action );
}

QPtrList<KAction> *Plugin::newActions() const
{
  return d->newActions;
}

Kontact::Core *Plugin::core() const
{
  return d->core;
}

void Plugin::select()
{
}

void Plugin::configUpdated()
{
}

void Plugin::partDestroyed()
{
  d->part = 0;
}

void Plugin::slotConfigUpdated()
{
  configUpdated();
}

void Plugin::bringToForeground()
{
  if (!d->executableName.isEmpty())
    KRun::runCommand(d->executableName);
}

bool Kontact::Plugin::showInSideBar() const
{
  return d->hasPart;
}

void Kontact::Plugin::setShowInSideBar( bool hasPart )
{
  d->hasPart = hasPart;
}

void Plugin::virtual_hook( int, void* ) {
	//BASE::virtual_hook( id, data );
}

#include "plugin.moc"

// vim: sw=2 et sts=2 tw=80
