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

using namespace Kontact;

Core::Core( QWidget *parent, const char *name )
  : KParts::MainWindow( parent, name )
{
}

Core::~Core()
{
}

KParts::ReadOnlyPart *Core::createPart( const char *libname )
{
  kdDebug() << "Core:createPart(): " << libname << endl; 

  QMap<QCString,KParts::ReadOnlyPart *>::ConstIterator it;
  it = mParts.find( libname );
  if ( it != mParts.end() ) return it.data();

  kdDebug() << "Creating new KPart" << endl;

  KParts::ReadOnlyPart *part =
      KParts::ComponentFactory::
          createPartInstanceFromLibrary<KParts::ReadOnlyPart>
              ( libname, this, 0, this, "kontact" );

  if ( part ) mParts.insert( libname, part );

  return part;
}
