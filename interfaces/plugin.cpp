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

#include "libkdepim/aboutdataextension.h"
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
};


Plugin::Plugin( Kontact::Core *core, QObject *parent, const char *name )
  : QObject( parent, name ), d( new Private )
{
  d->core = core;
  d->dcopClient = 0;
  d->newActions = new QPtrList<KAction>; 
}


Plugin::~Plugin()
{
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

KAboutData *Plugin::aboutData()
{
  KParts::Part *p = part();

  if ( !p )
    return 0;

  QObjectList *list = p->queryList( "KParts::AboutDataExtension" );
  KParts::AboutDataExtension *about =
          static_cast<KParts::AboutDataExtension*>( list->first() );

  if ( !about )
    return 0;
  else
    return about->aboutData();
}

DCOPClient *Plugin::dcopClient() const
{
  if ( !d->dcopClient ) {
    d->dcopClient = new DCOPClient();
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

#include "plugin.moc"
