/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <libkcal/event.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>

#include "core.h"
#include "plugin.h"
#include "korganizerplugin.h"

#include "summarywidget.h"

SummaryWidget::SummaryWidget( KOrganizerPlugin *plugin, QWidget *parent,
                              const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "korganizer", 
                   KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Appointments" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout( mainLayout, 7, 5, 3 );
  mLayout->setRowStretch( 6, 1 );

  KConfig config( "korganizerrc" );
  mCalendar = new KCal::CalendarResources( config.readEntry( "TimeZoneId" ) );
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  mCalendar->readConfig();
  if ( manager->isEmpty() ) {
    config.setGroup( "General" );
    QString fileName = config.readPathEntry( "Active Calendar" );

    QString resourceName;
    if ( fileName.isEmpty() ) {
      fileName = locateLocal( "data", "korganizer/std.ics" );
      resourceName = i18n( "Default KOrganizer resource" );
    } else {
      resourceName = i18n( "Active Calendar" );
    }

    KCal::ResourceCalendar *defaultResource =
                             new KCal::ResourceLocal( fileName );

    defaultResource->setResourceName( resourceName );

    manager->add( defaultResource );
    manager->setStandardResource( defaultResource );
  }

  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );

  updateView();
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KIconLoader loader( "korganizer" );

  QLabel *label = 0;
  int counter = 0;
  KCal::Event::List events = mCalendar->events( QDate::currentDate(), true );
  if ( events.count() > 0 ) {
    QPixmap pm = loader.loadIcon( "appointment", KIcon::Small );

    KCal::Event::List::ConstIterator it;
    for( it = events.begin(); it != events.end() && counter < 5; ++it ) {
      KCal::Event *ev = *it;
      if ( ev->doesFloat() && ev->dtStart().date() < QDate::currentDate() )
        continue;

      if ( ev->recurrence()->doesRecur() && !ev->recursOn( QDate::currentDate() ) )
        continue;

      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumSize( label->minimumSizeHint() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      QString date = ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
      label = new QLabel( date, this );
      label->setAlignment( AlignHCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      KURLLabel *urlLabel = new KURLLabel( ev->uid(), ev->summary(), this );
      mLayout->addWidget( urlLabel, counter, 2 );
      mLabels.append( urlLabel );
          
      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( selectEvent( const QString& ) ) );

      counter++;
    }
  } else {
    QLabel *noEvents = new QLabel( i18n( "No appointments pending" ), this );
    noEvents->setAlignment( AlignRight );
    mLayout->addWidget( noEvents, 0, 2 );
    mLabels.append( noEvents );
  }

  show();
}

void SummaryWidget::selectEvent( const QString & )
{
  mPlugin->interface()->showEventView();
}

#include "summarywidget.moc"
