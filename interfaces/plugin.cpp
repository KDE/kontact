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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QObject>
#include <QDBusConnection>
#include <kxmlguifactory.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <krun.h>

#include "core.h"
#include "plugin.h"

using namespace Kontact;

class Plugin::Private
{
  public:
    Kontact::Core *core;
    QList<KAction*> *newActions;
    QList<KAction*> *syncActions;
    QString identifier;
    QString title;
    QString icon;
    QString executableName;
    QString serviceName;
    QByteArray partLibraryName;
    bool hasPart;
    KParts::ReadOnlyPart *part;
    bool disabled;
};


Plugin::Plugin( Kontact::Core *core, QObject *parent, const char *name )
  : KXMLGUIClient(  core ), QObject(  parent ), d(  new Private )
{
  setObjectName( name );
  core->factory()->addClient( this );
  KGlobal::locale()->insertCatalog(name);

  d->core = core;
  d->newActions = new QList<KAction*>;
  d->syncActions = new QList<KAction*>;
  d->hasPart = true;
  d->part = 0;
  d->disabled = false;
}


Plugin::~Plugin()
{
  delete d->part;
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

void Plugin::setPartLibraryName( const QByteArray &libName )
{
  d->partLibraryName = libName;
}

KParts::ReadOnlyPart *Plugin::loadPart()
{
  return core()->createPart( d->partLibraryName );
}

const KAboutData *Plugin::aboutData()
{
  kDebug(5601) << "Plugin::aboutData(): libname: " << d->partLibraryName << endl;

  const KComponentData instance =
    KParts::Factory::partComponentDataFromLibrary( d->partLibraryName );

  if ( instance.isValid() ) {
    return instance.aboutData();
  } else {
    kError() << "Plugin::aboutData(): Can't load instance for "
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
  return QString();
}

QString Plugin::registerClient()
{
  if( d->serviceName.isEmpty())
  {
      d->serviceName = "org.kde." + objectName().toLatin1();
      QDBusConnection::sessionBus().registerService( d->serviceName );
  }
  return d->serviceName;
}

void Plugin::insertNewAction( KAction *action )
{
  d->newActions->append( action );
}

void Plugin::insertSyncAction( KAction *action )
{
  d->syncActions->append( action );
}

QList<KAction*> *Plugin::newActions() const
{
  return d->newActions;
}

QList<KAction*> *Plugin::syncActions() const
{
  return d->syncActions;
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
    KRun::runCommand(d->executableName,0);
}

bool Kontact::Plugin::showInSideBar() const
{
  return d->hasPart;
}

void Kontact::Plugin::setShowInSideBar( bool hasPart )
{
  d->hasPart = hasPart;
}

void Kontact::Plugin::setDisabled( bool disabled )
{
    d->disabled = disabled;
}

bool Kontact::Plugin::disabled() const
{
    return d->disabled;
}

void Plugin::virtual_hook( int, void* ) {
	//BASE::virtual_hook( id, data );
}

#include "plugin.moc"

// vim: sw=2 et sts=2 tw=80
