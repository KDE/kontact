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
	  : Kontact::Summary( parent, name ), mPlugin( plugin ), mCalendar( 0 )
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

  config.setGroup( "Date & Time" );
  mCalendar->setTimeZoneId( config.readEntry( "TimeZoneId" ) );
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           SLOT( updateView() ) );

  updateView();
}

SummaryWidget::~SummaryWidget()
{
  delete mCalendar;
  mCalendar = 0;
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KIconLoader loader( "korganizer" );

  KConfig config( "kcmkorgsummaryrc" );

  config.setGroup( "Calendar" );
  int days = config.readNumEntry( "DaysToShow", 1 );

  QLabel *label = 0;
  int counter = 0;
  KCal::Event::List events = mCalendar->events( QDate::currentDate(),
                                                QDate::currentDate().addDays( days - 1 ), true );

  // sort events
  KCal::Event::List sortedEvents;
  KCal::Event::List::Iterator sortIt;
  KCal::Event::List::Iterator eventIt;
  for ( eventIt = events.begin(); eventIt != events.end(); ++eventIt ) {
    sortIt = sortedEvents.begin();
    while ( sortIt != sortedEvents.end() &&
            (*eventIt)->dtStart().date() >= (*sortIt)->dtStart().date() ) {
      ++sortIt;
    }
    sortedEvents.insert( sortIt, *eventIt );
  }

  // display events
  if ( events.count() > 0 ) {
    QPixmap pm = loader.loadIcon( "appointment", KIcon::Small );

    KCal::Event::List::ConstIterator it;
    for( it = sortedEvents.begin(); it != sortedEvents.end() && counter < 5; ++it ) {
      KCal::Event *ev = *it;

      if ( days == 1 ) {
        if ( ev->doesFloat() && ev->dtStart().date() < QDate::currentDate() )
          continue;

        if ( ev->recurrence()->doesRecur() && !ev->recursOn( QDate::currentDate() ) )
          continue;
      }

      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumSize( label->minimumSizeHint() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      QString date;
      if ( days > 1 )
        date += ev->dtStartDateStr() + ": ";
      date += ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
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

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void SummaryWidget::selectEvent( const QString & )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" );
  mPlugin->interface()->showEventView();
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkorgsummary.desktop" );
}

#include "summarywidget.moc"
