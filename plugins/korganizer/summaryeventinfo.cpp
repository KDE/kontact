/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008 Allen Winter <winter@kde.org>
  Copyright (c) 2008 Thomas McGuire <mcguire@kde.org>

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

#include "summaryeventinfo.h"

#include <libkdepim/kpimprefs.h>
#include <korganizer/stdcalendar.h>
#include <kcal/incidenceformatter.h>
#include <kglobal.h>
#include <klocale.h>

#include <QDate>
#include <QStringList>

bool SummaryEventInfo::mShowBirthdays = true;
bool SummaryEventInfo::mShowAnniversaries = true;

void SummaryEventInfo::setShowSpecialEvents( bool showBirthdays,
                                             bool showAnniversaries )
{
  mShowBirthdays = showBirthdays;
  mShowAnniversaries = showAnniversaries;
}

bool SummaryEventInfo::skip( KCal::Event *event )
{
  //simply check categories because the birthdays resource always adds
  //the appropriate category to the event.
  QStringList c = event->categories();
  if ( !mShowBirthdays &&
       c.contains( i18n( "BIRTHDAY" ), Qt::CaseInsensitive ) ) {
    return true;
  }
  if ( !mShowAnniversaries &&
       c.contains( i18n( "ANNIVERSARY" ), Qt::CaseInsensitive ) ) {
    return true;
  }

  return false;
}

void SummaryEventInfo::dateDiff( const QDate &date, int &days )
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
    if ( QDate::isLeapYear( QDate::currentDate().year() ) ) {
      days++;
    }
  } else {
    days = offset;
  }
}

SummaryEventInfo::SummaryEventInfo()
  : makeBold( false )
{
}

SummaryEventInfo::List SummaryEventInfo::eventsForDate( const QDate &date,
                                                        KCal::Calendar *calendar )
{
  KCal::Event *ev;

  KCal::Event::List events_orig = calendar->events( date, calendar->timeSpec() );
  KCal::Event::List::ConstIterator it = events_orig.constBegin();

  KCal::Event::List events;
  events.setAutoDelete( true );
  KDateTime qdt;
  KDateTime::Spec spec = KPIM::KPimPrefs::timeSpec();
  QDate currentDate = QDate::currentDate();

  // prevent implicitely sharing while finding recurring events
  // replacing the QDate with the currentDate
  for ( ; it != events_orig.constEnd(); ++it ) {
    ev = (*it)->clone();
    if ( ev->recursOn( date, calendar->timeSpec() ) ) {
      qdt = ev->dtStart();
      qdt.setDate( date );
      ev->setDtStart( qdt );
    }
    if ( !skip( ev ) ) {
      events.append( ev );
    }
  }

  // sort the events for this date by summary
  events = KCal::Calendar::sortEvents( &events,
                                       KCal::EventSortSummary,
                                       KCal::SortDirectionAscending );
  // sort the events for this date by start date
  events = KCal::Calendar::sortEvents( &events,
                                       KCal::EventSortStartDate,
                                       KCal::SortDirectionAscending );

  List eventInfoList;

  for ( it=events.constBegin(); it != events.constEnd(); ++it ) {
    ev = *it;
    int daysTo = -1;

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
        if ( d < date ) {
          dayof++;
        }
        span++;
        d = d.addDays( 1 );
      }
    }

    QDate startOfMultiday = ev->dtStart().date();
    if ( startOfMultiday < currentDate ) {
      startOfMultiday = currentDate;
    }
    bool firstDayOfMultiday = ( date == startOfMultiday );

    // If this date is part of a floating, multiday event, then we
    // only make a print for the first day of the event.
    if ( ev->isMultiDay() && ev->allDay() &&
         ( currentDate > ev->dtStart().date() || !firstDayOfMultiday ) ) {
      continue;
    }

    SummaryEventInfo *summaryEvent = new SummaryEventInfo();
    eventInfoList.append( summaryEvent );

    // Start date label
    QString str = "";
    QDate sD = QDate( date.year(), date.month(), date.day() );
    if ( ( sD.month() == currentDate.month() ) &&
          ( sD.day()   == currentDate.day() ) ) {
      str = i18nc( "the appointment is today", "Today" );
      summaryEvent->makeBold = true;
    } else if ( ( sD.month() == currentDate.addDays( 1 ).month() ) &&
                ( sD.day()   == currentDate.addDays( 1 ).day() ) ) {
      str = i18nc( "the appointment is tomorrow", "Tomorrow" );
    } else {
      str = KGlobal::locale()->formatDate( sD, KLocale::FancyLongDate );
    }
    summaryEvent->startDate = str;

    // Print the date span for multiday, floating events, for the
    // first day of the event only.
    if ( ev->isMultiDay() && ev->allDay() && firstDayOfMultiday && span > 1 ) {
      str = ev->dtStartDateStr( false, spec ) +
            " -\n " +
            ev->dtEndDateStr( false, spec );
    }
    summaryEvent->dateSpan = str;

    // Days to go label
    str = "";
    dateDiff( startOfMultiday, daysTo );
    if ( ev->isMultiDay() && !ev->allDay() ) {
      dateDiff( date, daysTo );
    }
    if ( daysTo > 0 ) {
      str = i18np( "in 1 day", "in %1 days", daysTo );
    } else {
      str = i18n( "now" );
    }
    summaryEvent->daysToGo = str;

    // Summary label
    str = ev->richSummary();
    if ( ev->isMultiDay() &&  !ev->allDay() ) {
      str.append( QString( " (%1/%2)" ).arg( dayof ).arg( span ) );
    }
    summaryEvent->summaryText = str;
    summaryEvent->summaryUrl = ev->uid();
    QString tipText(
      KCal::IncidenceFormatter::toolTipStr( ev, true, KPIM::KPimPrefs::timeSpec() ) );
    if ( !tipText.isEmpty() ) {
      summaryEvent->summaryTooltip = tipText;
    }

    // Time range label (only for non-floating events)
    str = "";
    if ( !ev->allDay() ) {
      QTime sST = ev->dtStart().toTimeSpec( spec ).time();
      QTime sET = ev->dtEnd().toTimeSpec( spec ).time();
      if ( ev->isMultiDay() ) {
        if ( ev->dtStart().date() < date ) {
          sST = QTime( 0, 0 );
        }
        if ( ev->dtEnd().date() > date ) {
          sET = QTime( 23, 59 );
        }
      }
      str = i18nc( "Time from - to", "%1 - %2",
                    KGlobal::locale()->formatTime( sST ),
                    KGlobal::locale()->formatTime( sET ) );
      summaryEvent->timeRange = str;
    }
  }

  return eventInfoList;
}
