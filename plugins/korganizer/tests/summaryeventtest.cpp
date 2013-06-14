/* This file is part of the KDE project
   Copyright 2008 Thomas McGuire <mcguire@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"
#include "summaryeventtest.h"
#include "../summaryeventinfo.h"

#include <kcalcore/event.h>
#include <kcalcore/memorycalendar.h>

QTEST_KDEMAIN_CORE( SummaryEventTester )

void SummaryEventTester::test_Multiday()
{
  QDate today = QDate::currentDate();
  QString multidayWithTimeInProgress = "Multiday, time specified, in progress";

  KCalCore::MemoryCalendar::Ptr cal( new KCalCore::MemoryCalendar( KDateTime().timeSpec() ) );

  KCalCore::Event::Ptr event( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays( -1 ) ) );
  event->setDtEnd( KDateTime( today.addDays( 5 ) ) );
  event->setSummary( "Multiday, allday, in progress (day 2/6)" );
  QVERIFY( cal->addEvent( event ) );

  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays( -1 ), QTime::fromString("12:00","hh:mm") ) );
  event->setDtEnd( KDateTime( today.addDays( 5 ), QTime::fromString("12:00","hh:mm") ) );
  event->setSummary( multidayWithTimeInProgress  );
  QVERIFY( cal->addEvent( event ) );
    for ( int i = 0; i < 5; ++i ) {
    SummaryEventInfo::List events4 = SummaryEventInfo::eventsForDate( today.addDays( i ), cal );
    QCOMPARE( 1, events4.size() );
    SummaryEventInfo *ev4 = events4.at(0);

    QCOMPARE( ev4->summaryText, QString(multidayWithTimeInProgress + " (%1/7)").arg(i+2));
    QCOMPARE( ev4->timeRange, QString("00:00 - 23:59") );
//    QCOMPARE( ev4->startDate, KGlobal::locale()->formatDate( QDate( today.addDays( i ) ), KLocale::FancyLongDate ) );
    QCOMPARE( ev4->makeBold, i == 0 );

    qDeleteAll( events4 );
  }

  // Test date a multiday event in the future has to correct DaysTo set
  QString multiDayWithTimeFuture = "Multiday, with time, in the future";
  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays( 100 ), QTime::fromString("12:00","hh:mm") ) );
  event->setDtEnd( KDateTime( today.addDays( 106 ), QTime::fromString("12:00","hh:mm") ) );
  event->setSummary( multiDayWithTimeFuture );
  QVERIFY( cal->addEvent( event ) );
  for ( int i = 100; i <= 106; ++i ) {
    SummaryEventInfo::List events5 = SummaryEventInfo::eventsForDate( today.addDays( i ), cal );
    QCOMPARE( 1, events5.size() );
    SummaryEventInfo *ev5 = events5.at(0);
    /*qDebug() << ev5->summaryText;
    qDebug() << ev5->daysToGo;
    qDebug() << i;*/

    QCOMPARE( ev5->summaryText, QString(multiDayWithTimeFuture + " (%1/7)").arg(i-100+1));
    QCOMPARE( ev5->daysToGo, QString("in %1 days").arg(i) );

    qDeleteAll( events5 );
  }

  QString multiDayAllDayInFuture = "Multiday, allday, in future";
  int multiDayFuture = 30;
  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays( multiDayFuture ) ) );
  event->setDtEnd( KDateTime( today.addDays( multiDayFuture + 5 ) ) );
  event->setSummary( multiDayAllDayInFuture );
  QVERIFY( cal->addEvent( event ) );

  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays( 2 ), QTime::fromString("12:00","hh:mm") ) );
  event->setDtEnd( KDateTime( today.addDays( 5 ), QTime::fromString("12:00","hh:mm") ) );
  event->setSummary( "Multiday, time specified, in future" );
  QVERIFY( cal->addEvent( event ) );

  QString multiDayAllDayStartingToday = "Multiday, allday, starting today";
  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today ) );
  event->setDtEnd( KDateTime( today.addDays( 5 ) ) );
  event->setSummary( multiDayAllDayStartingToday );
  QVERIFY( cal->addEvent( event ) );

  event = KCalCore::Event::Ptr( new KCalCore::Event() );
  event->setDtStart( KDateTime( today.addDays(-10), QTime::fromString("12:00","hh:mm") ) );
  event->setDtEnd( KDateTime( today.addDays( -5 ), QTime::fromString("10:00","hh:mm") ) );
  event->setSummary( "Some event in the past" );
  QVERIFY( cal->addEvent( event ) );

  SummaryEventInfo::List eventsToday = SummaryEventInfo::eventsForDate( today, cal );
  QCOMPARE( 2, eventsToday.size() );
  foreach( const SummaryEventInfo *ev, eventsToday ) {
    if ( ev->summaryText == multidayWithTimeInProgress + " (2/7)" ) {
      QCOMPARE( ev->timeRange, QString("00:00 - 23:59") );
      QCOMPARE( ev->startDate, QString("Today") );
      QCOMPARE( ev->daysToGo, QString("now") );
      QCOMPARE( ev->makeBold, true );
    }
    else if ( ev->summaryText == multiDayAllDayStartingToday ) {
      QVERIFY( ev->timeRange.isEmpty() );
      QCOMPARE( ev->startDate, QString("Today") );
      QCOMPARE( ev->daysToGo, QString("all day") );
      QCOMPARE( ev->makeBold, true );
    }
    else
      Q_ASSERT( false ); // unexpected event!
  }

  SummaryEventInfo::List events2 = SummaryEventInfo::eventsForDate( today.addDays( multiDayFuture ), cal );
  QCOMPARE( 1, events2.size() );
  SummaryEventInfo *ev1 = events2.at( 0 );
  QCOMPARE( ev1->summaryText, multiDayAllDayInFuture );
  QVERIFY( ev1->timeRange.isEmpty() );
  QCOMPARE( ev1->startDate, KGlobal::locale()->formatDate( QDate( today.addDays( multiDayFuture ) ) ) );
  QCOMPARE( ev1->daysToGo, QString("in %1 days").arg(multiDayFuture) );
  QCOMPARE( ev1->makeBold, false );
  // Make sure multiday is only displayed once
  for ( int i = 1; i < 30; ++i ) {
    SummaryEventInfo::List events3 = SummaryEventInfo::eventsForDate( today.addDays( multiDayFuture + i ), cal );
    foreach(SummaryEventInfo *ev, events3 ) {
      QVERIFY( ev->summaryText.contains( multiDayAllDayInFuture ) );
    }
    qDeleteAll( events3 );
  }

  qDeleteAll( eventsToday );
  qDeleteAll( events2 );
}

#include "summaryeventtest.moc"
