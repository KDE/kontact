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

using namespace Kontact;

Core::Core( QWidget *parent, const char *name )
  : KParts::MainWindow( parent, name )
{
  QTimer* timer = new QTimer( this );
  mLastDate = QDate::currentDate();
  connect(timer, SIGNAL( timeout() ), SLOT( checkNewDay() ) );
  timer->start( 1000*60 );
}

Core::~Core()
{
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
    if ( error == KParts::ComponentFactory::ErrNoLibrary ) {
      // ### how to pass it to kontact for displaying together with the "Cannot load Part" box?
      kdWarning(5601) << KLibLoader::self()->lastErrorMessage() << endl;
    } else {
      kdWarning(5601) << "KParts::ComponentFactory::createInstanceFromFactory returned error code " << error << " for " << libname << endl;
    }
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

#include "core.moc"
// vim: sw=2 sts=2 et tw=80
