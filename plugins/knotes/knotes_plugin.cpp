/*
       This file is part of Kontact
       Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

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

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kstatusbar.h>

#include "core.h"
#include "knotes_part.h"
#include "summarywidget.h"

#include "knotes_plugin.h"

typedef KGenericFactory< KNotesPlugin, Kontact::Core > KNotesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpknotesplugin,
                            KNotesPluginFactory( "kpknotesplugin" ) )

KNotesPlugin::KNotesPlugin( Kontact::Core *core, const char *,
                            const QStringList &)
  : Kontact::Plugin( core, core, "knotes" ), 
    mPart( 0 )
{
  setInstance( KNotesPluginFactory::instance() );

  setXMLFile( "kpknotesplugin.rc" );

  insertNewAction( new KAction( i18n( "New Note" ), BarIcon( "knotes" ), 0,
                   this, SLOT( slotNewNote() ), actionCollection(),
                   "new_note" ) );
}

KNotesPlugin::~KNotesPlugin()
{
}

KParts::Part* KNotesPlugin::part()
{
  if ( !mPart )
    mPart = new KNotesPart( this, "notes" );

  return mPart;
}

Kontact::Summary *KNotesPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new SummaryWidget( parentWidget );
}

void KNotesPlugin::slotNewNote()
{
  (void) part();
  if ( mPart )
      mPart->newNote();
}

#include "knotes_plugin.moc"
