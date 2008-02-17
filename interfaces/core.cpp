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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "core.h"

#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <QTimer>
#include <klocale.h>

using namespace Kontact;

class Core::Private
{
  Core* const q;
public:
  explicit Private( Core* qq );

  void slotPartDestroyed( QObject * );
  void checkNewDay();

  QString lastErrorMessage;
  QDate mLastDate;
  QMap<QByteArray,KParts::ReadOnlyPart *> mParts;
};

Core::Private::Private( Core* qq ) : q( qq ), mLastDate( QDate::currentDate() )
{
}

Core::Core( QWidget *parent, Qt::WindowFlags f )
  : KParts::MainWindow( parent, f ), d( new Private( this ) )
{
  QTimer* timer = new QTimer( this );
  connect(timer, SIGNAL( timeout() ), SLOT( checkNewDay() ) );
  timer->start( 1000*60 );
}

Core::~Core()
{
  delete d;
}

KParts::ReadOnlyPart *Core::createPart( const char *libname )
{
  kDebug(5601) << libname;

  QMap<QByteArray,KParts::ReadOnlyPart *>::ConstIterator it;
  it = d->mParts.find( libname );
  if ( it != d->mParts.end() ) return it.value();

  kDebug(5601) <<"Creating new KPart";

  KPluginLoader loader( libname );
  kDebug(5601) << loader.fileName();
  KPluginFactory *factory = loader.factory();
  KParts::ReadOnlyPart *part = 0;
  if (factory)
    part = factory->create<KParts::ReadOnlyPart>(this);

  if (part) {
    d->mParts.insert( libname, part );
    QObject::connect( part, SIGNAL( destroyed( QObject * ) ),
                      SLOT( slotPartDestroyed( QObject * ) ) );
  } else {
    d->lastErrorMessage = loader.errorString();
    kWarning(5601) << d->lastErrorMessage;
  }

  return part;
}

void Core::Private::slotPartDestroyed( QObject * obj )
{
  // the part was deleted, we need to remove it from the part map to not return
  // a dangling pointer in createPart
  QMap<QByteArray, KParts::ReadOnlyPart*>::Iterator end = mParts.end();
  QMap<QByteArray, KParts::ReadOnlyPart*>::Iterator it = mParts.begin();
  for ( ; it != end; ++it ) {
    if ( it.value() == obj ) {
      mParts.erase( it );
      return;
    }
  }
}

void Core::Private::checkNewDay()
{
  if ( mLastDate != QDate::currentDate() )
    emit q->dayChanged( QDate::currentDate() );

  mLastDate = QDate::currentDate();
}

QString Core::lastErrorMessage() const
{
  return d->lastErrorMessage;
}

#include "core.moc"
// vim: sw=2 sts=2 et tw=80
