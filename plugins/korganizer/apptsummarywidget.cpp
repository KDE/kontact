/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2005-2006 Allen Winter <winter@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <qtooltip.h>
#include <libkcal/event.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkdepim/kpimprefs.h>

#include "korganizeriface_stub.h"

#include "core.h"
#include "plugin.h"
#include "korganizerplugin.h"

#include "korganizer/stdcalendar.h"

#include "apptsummarywidget.h"

ApptSummaryWidget::ApptSummaryWidget( KOrganizerPlugin *plugin, QWidget *parent,
                                      const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin ), mCalendar( 0 )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_date",
                   KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Upcoming Events" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout( mainLayout, 7, 5, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           SLOT( updateView() ) );

  updateView();
}

ApptSummaryWidget::~ApptSummaryWidget()
{
}

void ApptSummaryWidget::updateView()
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig config( "kcmapptsummaryrc" );
  config.setGroup( "Days" );
  int days = config.readEntry( "DaysToShow", 7 );

  // The event print consists of the following fields:
  //  icon:start date:days-to-go:summary:time range
  // where,
  //   the icon is the typical event icon
  //   the start date is the event start date
  //   the days-to-go is the #days until the event starts
  //   the summary is the event summary
  //   the time range is the start-end time (only for non-floating events)

  // No reason to show the date year
  QString savefmt = KGlobal::locale()->dateFormat();
  KGlobal::locale()->setDateFormat( KGlobal::locale()->
                                    dateFormat().replace( 'Y', ' ' ) );

  QLabel *label = 0;
  int counter = 0;

  KIconLoader loader( "korganizer" );
  QPixmap pm = loader.loadIcon( "appointment", KIcon::Small );

  QString str;
  QDate dt;
  for ( dt=QDate::currentDate();
        dt<=QDate::currentDate().addDays( days - 1 );
        dt=dt.addDays(1) ) {
    KCal::Event::List events = mCalendar->events( dt );

    // sort the events for this date by summary
    events = KCal::Calendar::sortEvents( &events,
                                         KCal::EventSortSummary,
                                         KCal::SortDirectionAscending );
    // sort the events for this date by start date
    events = KCal::Calendar::sortEvents( &events,
                                         KCal::EventSortStartDate,
                                         KCal::SortDirectionAscending );

    KCal::Event *ev;
    KCal::Event::List::ConstIterator it;
    for ( it=events.begin(); it!=events.end(); ++it ) {
      ev = *it;
      bool makeBold = false;
      int daysTo = -1;

      // Count number of days remaining in multiday event
      int span=1; int dayof=1;
      if ( ev->isMultiDay() ) {
        QDate d = ev->dtStart().date();
        if ( d < QDate::currentDate() ) {
          d = QDate::currentDate();
        }
        while ( d < ev->dtEnd().date() ) {
          if ( d < dt ) {
            dayof++;
          }
          span++;
          d=d.addDays( 1 );
        }
      }

      // If this date is part of a floating, multiday event, then we
      // only make a print for the first day of the event.
      if ( ev->isMultiDay() && ev->doesFloat() && dayof != 1 )break;

      // Icon label
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Start date label
      str = "";
      QDate sD = QDate::QDate( dt.year(), dt.month(), dt.day() );
      if ( ( sD.month() == QDate::currentDate().month() ) &&
           ( sD.day()   == QDate::currentDate().day() ) ) {
        str = i18n( "Today" );
        makeBold = true;
      } else if ( ( sD.month() == QDate::currentDate().addDays( 1 ).month() ) &&
                  ( sD.day()   == QDate::currentDate().addDays( 1 ).day() ) ) {
        str = i18n( "Tomorrow" );
      } else {
        str = KGlobal::locale()->formatDate( sD );
      }

      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( ev->isMultiDay() && ev->doesFloat() && dayof == 1 && span > 1 ) {
        str += " -\n " + KGlobal::locale()->formatDate( sD.addDays( span-1 ) );
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

      // Days togo label
      str = "";
      dateDiff( ev->dtStart().date(), daysTo );
      if ( daysTo > 0 ) {
        str = i18n( "in 1 day", "in %n days", daysTo );
      } else {
        str = i18n( "now" );
      }
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Summary label
      str = ev->summary();
      if ( ev->isMultiDay() &&  !ev->doesFloat() ) {
        str.append( QString(" (%1/%2)").arg( dayof ).arg( span ) );
      }

      KURLLabel *urlLabel = new KURLLabel( ev->uid(), str, this );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      mLayout->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );

      if ( !ev->description().isEmpty() ) {
        urlLabel->setToolTip( ev->description() );
      }

      // Time range label (only for non-floating events)
      str = "";
      if ( !ev->doesFloat() ) {
        QTime sST = ev->dtStart().time();
        QTime sET = ev->dtEnd().time();
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() < dt ) {
            sST = QTime::QTime( 0, 0 );
          }
          if ( ev->dtEnd().date() > dt ) {
            sET = QTime::QTime( 23, 59 );
          }
        }
        str = i18n( "Time from - to", "%1 - %2" )
              .arg( KGlobal::locale()->formatTime( sST ) )
              .arg( KGlobal::locale()->formatTime( sET ) );
        label = new QLabel( str, this );
        label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        mLayout->addWidget( label, counter, 4 );
        mLabels.append( label );
      }

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( selectEvent( const QString& ) ) );

      counter++;
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18n( "No upcoming events starting within the next day",
            "No upcoming events starting within the next %n days",
            days ), this, "nothing to see" );
    noEvents->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noEvents, 0, 2 );
    mLabels.append( noEvents );
  }

  Q_FOREACH( label, mLabels )
    label->show();

  KGlobal::locale()->setDateFormat( savefmt );
}

void ApptSummaryWidget::selectEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

bool ApptSummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Edit Event: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void ApptSummaryWidget::dateDiff( const QDate &date, int &days )
{
  QDate currentDate;
  QDate eventDate;

  if ( QDate::leapYear( date.year() ) && date.month() == 2 && date.day() == 29 ) {
    currentDate =
      QDate( date.year(), QDate::currentDate().month(), QDate::currentDate().day() );
    if ( !QDate::leapYear( QDate::currentDate().year() ) )
      eventDate = QDate( date.year(), date.month(), 28 );
    else
      eventDate = QDate( date.year(), date.month(), date.day() );
  } else {
    currentDate =
      QDate( 0, QDate::currentDate().month(), QDate::currentDate().day() );
    eventDate = QDate( 0, date.month(), date.day() );
  }

  int offset = currentDate.daysTo( eventDate );
  if ( offset < 0 ) {
    days = 365 + offset;
  } else {
    days = offset;
  }
}

QStringList ApptSummaryWidget::configModules() const
{
  return QStringList( "kcmapptsummary.desktop" );
}

#include "apptsummarywidget.moc"
