/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 David Faure <faure@kde.org>

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

#include "korg_uniqueapp.h"
#include "../../korganizer/korganizer_options.h"

#include "core.h"
#include <dcopref.h>
#include <kapplication.h>
#include <kstartupinfo.h>
#include <kwin.h>

void KOrganizerUniqueAppHandler::loadCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( korganizer_options );
}

int KOrganizerUniqueAppHandler::newInstance()
{
  // Ensure part is loaded
  (void)plugin()->part();
  DCOPRef korganizer( "korganizer", "KOrganizerIface" );
  korganizer.send( "handleCommandLine" );

  // Bring korganizer's plugin to front
  // This bit is duplicated from KUniqueApplication::newInstance()
  if ( kapp->mainWidget() ) {
    kapp->mainWidget()->show();
    KWin::forceActiveWindow( kapp->mainWidget()->winId() );
    KStartupInfo::appStarted();
  }

  // Then ensure the part appears in kontact.
  // ALWAYS use the korganizer plugin; i.e. never show the todo nor journal
  // plugins when creating a new instance via the command line, even if
  // the command line options are empty; else we'd need to examine the
  // options and then figure out which plugin we should show.
  // kolab/issue3971
  plugin()->core()->selectPlugin( "kontact_korganizerplugin" );
  return 0;
}
