/*
    This file is part of KDE Kontact.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstaticdeleter.h>

#include "prefs.h"

using namespace Kontact;

Prefs *Prefs::mInstance = 0;

static KStaticDeleter<Prefs> insd;


Prefs::Prefs()
  : KConfigSkeleton( "kontactrc" )
{
  KConfigSkeleton::setCurrentGroup( "View" );

  QStringList values;
  values.append( "SidePaneBars" );
  values.append( "SidePaneIcons" );
  mSidePaneTypeItem =
      new KConfigSkeleton::ItemEnum( KConfigSkeleton::currentGroup(),
                                     "SidePaneType", mSidePaneType,
                                     values, SidePaneIcons );
  addItem( "", mSidePaneTypeItem );
  mSidePaneTypeItem->setLabel( i18n( "Side Pane Type" ) );

  addItemString( "", "ActivePlugin", mActivePlugin, "summary" );
  addItemIntList( "", "SidePaneSplitter", mSidePaneSplitter );
}

Prefs::~Prefs()
{
  if ( mInstance == this )
    mInstance = insd.setObject( 0 );
}

Prefs *Prefs::self()
{
  if ( !mInstance ) {
    mInstance = insd.setObject( new Prefs() );
    mInstance->readConfig();
  }

  return mInstance;
}
