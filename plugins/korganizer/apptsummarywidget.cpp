/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "apptsummarywidget.h"
#include "korganizerplugin.h"
#include "korganizerinterface.h"

#include <korganizer/stdcalendar.h>
#include <kontactinterfaces/core.h>

#include <kcal/incidenceformatter.h>
#include <kcal/resourcecalendar.h>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kurllabel.h>

#include <QDateTime>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextDocument>

ApptSummaryWidget::ApptSummaryWidget( KOrganizerPlugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ), mPlugin( plugin ), mCalendar( 0 )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QWidget *header = createHeader(
    this, "view-calendar-upcoming-events", i18n( "Upcoming Events" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();

  //If the kpart isn't created yet, it's created now, and mCalendar is loaded
  mPlugin->part();

  connect( mCalendar, SIGNAL(calendarChanged()), SLOT(updateView()) );
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)),
           SLOT(updateView()) );
  updateView();
}

ApptSummaryWidget::~ApptSummaryWidget()
{
}

void ApptSummaryWidget::updateView()
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig _config( "kcmapptsummaryrc" );
  KConfigGroup config(&_config, "Days" );
  int days = config.readEntry( "DaysToShow", 7 );

  // The event print consists of the following fields:
  //  icon:start date:days-to-go:summary:time range
  // where,
  //   the icon is the typical event icon
  //   the start date is the event start date
  //   the days-to-go is the #days until the event starts
  //   the summary is the event summary
  //   the time range is the start-end time (only for non-floating events)

  QLabel *label = 0;
  int counter = 0;

  KIconLoader loader( "korganizer" );
  QPixmap pm = loader.loadIcon( "view-calendar-day", KIconLoader::Small );

  QString str;
  QDate dt;
  QDate currentDate = QDate::currentDate();
  for ( dt=currentDate;
        dt<=currentDate.addDays( days - 1 );
        dt=dt.addDays(1) ) {

    KCal::Event *ev;

    KCal::Event::List events_orig = mCalendar->events( dt, mCalendar->timeSpec() );
    KCal::Event::List::ConstIterator it = events_orig.begin();

    KCal::Event::List events;
    events.setAutoDelete( true );
    KDateTime qdt;

    // prevent implicitely sharing while finding recurring events
    // replacing the QDate with the currentDate
    for ( ; it != events_orig.end(); ++it ) {
      ev = (*it)->clone();
      if ( ev->recursOn( dt, mCalendar->timeSpec() ) ) {
        qdt = ev->dtStart();
        qdt.setDate( dt );
        ev->setDtStart( qdt );
      }
      events.append( ev );
    }

    // sort the events for this date by summary
    events = KCal::Calendar::sortEvents( &events,
                                         KCal::EventSortSummary,
                                         KCal::SortDirectionAscending );
    // sort the events for this date by start date
    events = KCal::Calendar::sortEvents( &events,
                                         KCal::EventSortStartDate,
                                         KCal::SortDirectionAscending );

    for ( it=events.begin(); it != events.end(); ++it ) {
      ev = *it;
      bool makeBold = false;
      int daysTo = -1;

      // ### If you change anything in the multiday logic below, make sure
      //     you test it with:
      //     - a multiday, all-day event currently in progress
      //     - a multiday, all-day event in the future
      //     - a multiday event with associated time which is in progress
      //     - a multiday event with associated time which is in the future

      // Count number of days remaining in multiday event
      int span = 1;
      int dayof = 1;
      if ( ev->isMultiDay() ) {
        QDate d = ev->dtStart().date();
        if ( d < currentDate ) {
          dayof += d.daysTo( currentDate );
          span += d.daysTo( currentDate );
          d = currentDate;
        }
        while ( d < ev->dtEnd().date() ) {
          if ( d < dt ) {
            dayof++;
          }
          span++;
          d = d.addDays( 1 );
        }
      }

      QDate startOfMultiday = ev->dtStart().date();
      if ( startOfMultiday < currentDate )
        startOfMultiday = currentDate;
      bool firstDayOfMultiday = ( dt == startOfMultiday );

      // If this date is part of a floating, multiday event, then we
      // only make a print for the first day of the event.
    if ( ev->isMultiDay() && ev->allDay() &&
         ( currentDate > ev->dtStart().date() || !firstDayOfMultiday ) ) {
        continue;
      }

      // Icon label
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Start date label
      str = "";
      QDate sD = QDate( dt.year(), dt.month(), dt.day() );
      if ( ( sD.month() == currentDate.month() ) &&
           ( sD.day()   == currentDate.day() ) ) {
        str = i18nc( "@label the appointment is today", "Today" );
        makeBold = true;
      } else if ( ( sD.month() == currentDate.addDays( 1 ).month() ) &&
                  ( sD.day()   == currentDate.addDays( 1 ).day() ) ) {
        str = i18nc( "@label the appointment is tomorrow", "Tomorrow" );
      } else {
        str = KGlobal::locale()->formatDate( sD, KLocale::FancyLongDate );
      }

      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( ev->isMultiDay() && ev->allDay() && firstDayOfMultiday && span > 1 ) {
        str = KGlobal::locale()->formatDate( ev->dtStart().date(), KLocale::FancyLongDate );
        str += " -\n " +
               KGlobal::locale()->formatDate( ev->dtEnd().date(), KLocale::FancyLongDate );
      }

      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      // Days to go label
      str = "";
      dateDiff( startOfMultiday, daysTo );
      if ( ev->isMultiDay() && !ev->allDay() )
        dateDiff( dt, daysTo );
      if ( daysTo > 0 ) {
        str = i18np( "in 1 day", "in %1 days", daysTo );
      } else {
        str = i18n( "now" );
      }
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Summary label
      str = ev->richSummary();
      if ( ev->isMultiDay() &&  !ev->allDay() ) {
        str.append( QString( " (%1/%2)" ).arg( dayof ).arg( span ) );
      }

      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( str );
      urlLabel->setUrl( ev->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      urlLabel->setWordWrap( true );
      mLayout->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(viewEvent(const QString&)) );
      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(popupMenu(const QString&)) );

      QString tipText( KCal::IncidenceFormatter::toolTipString( ev, true ) );
      if ( !tipText.isEmpty() ) {
        urlLabel->setToolTip( tipText );
      }

      // Time range label (only for non-floating events)
      str = "";
      if ( !ev->allDay() ) {
        QTime sST = ev->dtStart().time();
        QTime sET = ev->dtEnd().time();
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() < dt ) {
            sST = QTime( 0, 0 );
          }
          if ( ev->dtEnd().date() > dt ) {
            sET = QTime( 23, 59 );
          }
        }
        str = i18nc( "Time from - to", "%1 - %2",
                     KGlobal::locale()->formatTime( sST ),
                     KGlobal::locale()->formatTime( sET ) );
        label = new QLabel( str, this );
        label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        mLayout->addWidget( label, counter, 4 );
        mLabels.append( label );
      }

      counter++;
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18np( "No upcoming events starting within the next day",
            "No upcoming events starting within the next %1 days",
            days ), this );
    noEvents->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noEvents, 0, 0 );
    mLabels.append( noEvents );
  }

  Q_FOREACH( label, mLabels )
  label->show();
}

void ApptSummaryWidget::viewEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.editIncidence( uid );
}

void ApptSummaryWidget::removeEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.deleteIncidence( uid, false );
}

void ApptSummaryWidget::popupMenu( const QString &uid )
{
  KMenu popup( this );
  QAction *editIt = popup.addAction( i18n( "&Edit Appointment..." ) );
  QAction *delIt = popup.addAction( i18n( "&Delete Appointment" ) );
  delIt->setIcon( KIconLoader::global()->
                  loadIcon( "edit-delete", KIconLoader::Small ) );

  const QAction *selectedAction = popup.exec( QCursor::pos() );
  if ( selectedAction == editIt ) {
    viewEvent( uid );
  } else if ( selectedAction == delIt ) {
    removeEvent( uid );
  }
}

bool ApptSummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel *label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Edit Event: \"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null ); //krazy:exclude=nullstrassign for old broken gcc
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void ApptSummaryWidget::dateDiff( const QDate &date, int &days )
{
  QDate currentDate;
  QDate eventDate;

  if ( QDate::isLeapYear( date.year() ) && date.month() == 2 && date.day() == 29 ) {
    currentDate = QDate( date.year(), QDate::currentDate().month(), QDate::currentDate().day() );
    if ( !QDate::isLeapYear( QDate::currentDate().year() ) ) {
      eventDate = QDate( date.year(), date.month(), 28 ); // celebrate one day earlier ;)
    } else {
      eventDate = QDate( date.year(), date.month(), date.day() );
    }
  } else {
    currentDate = QDate( QDate::currentDate().year(),
                         QDate::currentDate().month(),
                         QDate::currentDate().day() );
    eventDate = QDate( QDate::currentDate().year(), date.month(), date.day() );
  }

  int offset = currentDate.daysTo( eventDate );
  if ( offset < 0 ) {
    days = 365 + offset;
    if ( QDate::isLeapYear( QDate::currentDate().year() ) )
      days++;
  } else {
    days = offset;
  }
}

QStringList ApptSummaryWidget::configModules() const
{
  return QStringList( "kcmapptsummary.desktop" );
}

#include "apptsummarywidget.moc"
