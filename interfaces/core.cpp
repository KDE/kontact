/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "core.h"

#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <qtimer.h>
#include <klocale.h>

using namespace Kontact;

class Core::Private
{
public:
  QString lastErrorMessage;
};

Core::Core( QWidget *parent, const char *name )
  : KParts::MainWindow( parent, name )
{
  d = new Private;
  QTimer* timer = new QTimer( this );
  mLastDate = QDate::currentDate();
  connect(timer, SIGNAL( timeout() ), SLOT( checkNewDay() ) );
  timer->start( 1000*60 );
}

Core::~Core()
{
  delete d;
}

KParts::ReadOnlyPart *Core::createPart( const char *libname )
{
  kdDebug(5601) << "Core::createPart(): " << libname << endl;

  QMap<QCString,KParts::ReadOnlyPart *>::ConstIterator it;
  it = mParts.find( libname );
  if ( it != mParts.end() ) return it.data();

  kdDebug(5601) << "Creating new KPart" << endl;

  int error = 0;
  KParts::ReadOnlyPart *part =
      KParts::ComponentFactory::
          createPartInstanceFromLibrary<KParts::ReadOnlyPart>
              ( libname, this, 0, this, "kontact", QStringList(), &error );

  KParts::ReadOnlyPart *pimPart = dynamic_cast<KParts::ReadOnlyPart*>( part );
  if ( pimPart ) {
    mParts.insert( libname, pimPart );
    QObject::connect( pimPart, SIGNAL( destroyed( QObject * ) ),
                      SLOT( slotPartDestroyed( QObject * ) ) );
  } else {
    // TODO move to KParts::ComponentFactory
    switch( error ) {
    case KParts::ComponentFactory::ErrNoServiceFound:
      d->lastErrorMessage = i18n( "No service found" );
      break;
    case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
      d->lastErrorMessage = i18n( "Program error: the .desktop file for the service does not have a Library key." );
      break;
    case KParts::ComponentFactory::ErrNoLibrary:
      d->lastErrorMessage = KLibLoader::self()->lastErrorMessage();
      break;
    case KParts::ComponentFactory::ErrNoFactory:
      d->lastErrorMessage = i18n( "Program error: the library %1 does not provide a factory." ).arg( libname );
      break;
    case KParts::ComponentFactory::ErrNoComponent:
      d->lastErrorMessage = i18n( "Program error: the library %1 does not support creating components of the specified type" ).arg( libname );
      break;
    }
    kdWarning(5601) << d->lastErrorMessage << endl;
  }

  return pimPart;
}

void Core::slotPartDestroyed( QObject * obj )
{
  // the part was deleted, we need to remove it from the part map to not return
  // a dangling pointer in createPart
  QMap<QCString, KParts::ReadOnlyPart*>::Iterator end = mParts.end();
  QMap<QCString, KParts::ReadOnlyPart*>::Iterator it = mParts.begin();
  for ( ; it != end; ++it ) {
    if ( it.data() == obj ) {
      mParts.remove( it );
      return;
    }
  }
}

void Core::checkNewDay()
{
  if ( mLastDate != QDate::currentDate() )
    emit dayChanged( QDate::currentDate() );

  mLastDate = QDate::currentDate();
}

QString Core::lastErrorMessage() const
{
  return d->lastErrorMessage;
}

#include "core.moc"
// vim: sw=2 sts=2 et tw=80
