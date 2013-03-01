/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>
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

#include <KCalCore/Calendar>
#include <KCalCore/Event>
using namespace KCalCore;

#include <KCalUtils/IncidenceFormatter>
using namespace KCalUtils;

#include <KGlobal>
#include <KLocale>
#include <KSystemTimeZones>

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

bool SummaryEventInfo::skip( const KCalCore::Event::Ptr &event )
{
  //simply check categories because the birthdays resource always adds
  //the appropriate category to the event.
  QStringList c = event->categories();
  if ( !mShowBirthdays &&
       c.contains( "BIRTHDAY", Qt::CaseInsensitive ) ) {
    return true;
  }
  if ( !mShowAnniversaries &&
       c.contains( "ANNIVERSARY", Qt::CaseInsensitive ) ) {
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
                                                        const KCalCore::Calendar::Ptr &calendar )
{
  KCalCore::Event::Ptr ev;

  KCalCore::Event::List events = calendar->events( date, calendar->timeSpec() );

  KDateTime qdt;
  KDateTime::Spec spec = KSystemTimeZones::local();
  KDateTime currentDateTime = KDateTime::currentDateTime( spec );
  QDate currentDate = currentDateTime.date();

  // sort the events for this date by summary
  events = KCalCore::Calendar::sortEvents( events,
                                           KCalCore::EventSortSummary,
                                           KCalCore::SortDirectionAscending );
  // sort the events for this date by start date
  events = KCalCore::Calendar::sortEvents( events,
                                           KCalCore::EventSortStartDate,
                                           KCalCore::SortDirectionAscending );

  List eventInfoList;
  KCalCore::Event::List::ConstIterator end = events.constEnd();
  for ( KCalCore::Event::List::ConstIterator it=events.constBegin(); it != end; ++it ) {
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
    // If the event is already over, then it isn't upcoming. so don't print it.
    if ( !ev->allDay() ) {
      if ( ev->recurs() ) {
        KDateTime kdt( date, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
        kdt = kdt.addSecs( -1 );
        if ( currentDateTime > ev->recurrence()->getNextDateTime( kdt ) ) {
          continue;
        }
      } else {
        if ( currentDateTime > ev->dtEnd() ) {
          continue;
        }
      }
    }

    SummaryEventInfo *summaryEvent = new SummaryEventInfo();
    eventInfoList.append( summaryEvent );

    // Event
    summaryEvent->ev = ev;

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
      str = IncidenceFormatter::dateToString( ev->dtStart(), false, spec ) +
            " -\n " +
            IncidenceFormatter::dateToString( ev->dtEnd(), false, spec );
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
      if ( !ev->allDay() ) {
        int secs;
        if ( !ev->recurs() ) {
          secs = currentDateTime.secsTo( ev->dtStart() );
        } else {
          KDateTime kdt( date, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
          kdt = kdt.addSecs( -1 );
          KDateTime next = ev->recurrence()->getNextDateTime( kdt );
          secs = currentDateTime.secsTo( next );
        }
        if ( secs > 0 ) {
          str = i18nc( "eg. in 1 hour 2 minutes", "in " );
          int hours = secs / 3600;
          if ( hours > 0 ) {
            str += i18ncp( "use abbreviation for hour to keep the text short",
                           "1 hr", "%1 hrs", hours );
            str += ' ';
            secs -= ( hours * 3600 );
          }
          int mins = secs / 60;
          if ( mins > 0 ) {
            str += i18ncp( "use abbreviation for minute to keep the text short",
                           "1 min", "%1 mins", mins );
          }
        } else {
          str = i18n( "now" );
        }
      } else {
        str = i18n( "all day" );
      }
    }
    summaryEvent->daysToGo = str;

    // Summary label
    str = ev->richSummary();
    if ( ev->isMultiDay() &&  !ev->allDay() ) {
      str.append( QString( " (%1/%2)" ).arg( dayof ).arg( span ) );
    }
    summaryEvent->summaryText = str;
    summaryEvent->summaryUrl = ev->uid();
    QString tipText( KCalUtils::IncidenceFormatter::toolTipStr(
                       KCalUtils::IncidenceFormatter::resourceString(
                         calendar, ev ), ev, date, true, spec ) );
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

    // For recurring events, append the next occurrence to the time range label
    if ( ev->recurs() ) {
      KDateTime kdt( date, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
      kdt = kdt.addSecs( -1 );
      KDateTime next = ev->recurrence()->getNextDateTime( kdt );
      QString tmp = IncidenceFormatter::dateTimeToString(
        ev->recurrence()->getNextDateTime( next ), ev->allDay(),
        true, KSystemTimeZones::local() );
      if ( !summaryEvent->timeRange.isEmpty() ) {
        summaryEvent->timeRange += "<br>";
      }
      summaryEvent->timeRange += "<font size=\"small\"><i>" +
                                 i18nc( "next occurrence", "Next: %1", tmp ) +
                                 "</i></font>";
    }
  }

  return eventInfoList;
}
