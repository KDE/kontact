/*
    This file is part of Kontact.
    Copyright (c) 2003 Kontact Developer

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

#include <qwidget.h>
#include <qdragobject.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "core.h"
#include "summarywidget.h"

#include "korganizerplugin.h"

typedef KGenericFactory< KOrganizerPlugin, Kontact::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_korganizerplugin,
                            KOrganizerPluginFactory( "kontact_korganizerplugin" ) )

KOrganizerPlugin::KOrganizerPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ), 
    mIface( 0 )
{
  setInstance( KOrganizerPluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Event" ), BarIcon( "event" ),
                   0, this, SLOT( slotNewEvent() ), actionCollection(),
                   "new_event" ) );
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

Kontact::Summary *KOrganizerPlugin::createSummaryWidget( QWidget *parent )
{
  return new SummaryWidget( this, parent );
}

KParts::Part *KOrganizerPlugin::createPart()
{
  KParts::Part * part = loadPart();
  if ( !part ) return 0;

  (void) dcopClient(); // ensure that we register to DCOP as "korganizer"
  mIface = new KCalendarIface_stub( dcopClient(), "kontact", "CalendarIface" );

  return part;
}

QString KOrganizerPlugin::tipFile() const
{
  QString file = ::locate("data", "korganizer/tips");
  return file;
}

void KOrganizerPlugin::select()
{
  if ( mIface ) mIface->showEventView();
}

void KOrganizerPlugin::slotNewEvent()
{
  part();
  if ( !mIface )
    return;
}

bool KOrganizerPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool KOrganizerPlugin::canDecodeDrag( QMimeSource *mimeSource )
{
  return QTextDrag::canDecode( mimeSource );
}

void KOrganizerPlugin::processDropEvent( QDropEvent *event )
{
  QString text;
  if ( QTextDrag::decode( event, text ) ) {
    kdDebug() << "DROP:" << text << endl;
    if ( mIface ) mIface->openEventEditor( text );
    else kdError() << "KOrganizer Part not loaded." << endl;
  } else {
    KMessageBox::sorry( core(), i18n("Can't handle drop events of type '%1'.")
                                .arg( event->format() ) );
  }
}

#include "korganizerplugin.moc"
