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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcursor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <libkcal/event.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/incidenceformatter.h>
#include <libkdepim/kpimprefs.h>

#include "korganizeriface_stub.h"

#include "core.h"
#include "plugin.h"
#include "korganizerplugin.h"

#include "korganizer/stdcalendar.h"

#include "summarywidget.h"

SummaryWidget::SummaryWidget( KOrganizerPlugin *plugin, QWidget *parent,
                              const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin ), mCalendar( 0 )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_date",
                   KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Calendar" ) );
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

SummaryWidget::~SummaryWidget()
{
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KIconLoader loader( "kdepim" );

  KConfig config( "kcmkorgsummaryrc" );

  config.setGroup( "Calendar" );
  int days = config.readNumEntry( "DaysToShow", 1 );

  QLabel *label = 0;
  int counter = 0;
  QPixmap pm = loader.loadIcon( "appointment", KIcon::Small );

  QDate dt;
  QDate currentDate = QDate::currentDate();
  for ( dt=currentDate;
        dt<=currentDate.addDays( days - 1 );
        dt=dt.addDays(1) ) {

    KCal::Event *ev;

    KCal::Event::List events_orig = mCalendar->events( dt );
    KCal::Event::List::ConstIterator it = events_orig.begin();

    KCal::Event::List events;
    events.setAutoDelete( true );
    QDateTime qdt;

    // prevent implicitely sharing while finding recurring events
    // replacing the QDate with the currentDate
    for ( ; it != events_orig.end(); ++it ) {
      ev = (*it)->clone();
      if ( ev->recursOn( dt ) ) {
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

    for ( it=events.begin(); it!=events.end(); ++it ) {
      ev = *it;

      // Count number of days remaining in multiday event
      int span=1; int dayof=1;
      if ( ev->isMultiDay() ) {
        QDate d = ev->dtStart().date();
        if ( d < currentDate ) {
          d = currentDate;
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
      if ( ev->isMultiDay() && ev->doesFloat() && dayof != 1 ) continue;

      // Fill Appointment Pixmap Field
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignVCenter );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Fill Event Date Field
      bool makeBold = false;
      QString datestr;

      // Modify event date for printing
      QDate sD = QDate::QDate( dt.year(), dt.month(), dt.day() );
      if ( ( sD.month() == currentDate.month() ) &&
           ( sD.day()   == currentDate.day() ) ) {
        datestr = i18n( "Today" );
        makeBold = true;
      } else if ( ( sD.month() == currentDate.addDays( 1 ).month() ) &&
                  ( sD.day()   == currentDate.addDays( 1 ).day() ) ) {
        datestr = i18n( "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD );
      }

      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( ev->isMultiDay() && ev->doesFloat() && dayof == 1 && span > 1 ) {
        datestr = KGlobal::locale()->formatDate( ev->dtStart().date() );
        datestr += " -\n " +
                   KGlobal::locale()->formatDate( sD.addDays( span-1 ) );
      }

      label = new QLabel( datestr, this );
      label->setAlignment( AlignLeft | AlignVCenter );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      // Fill Event Summary Field
      QString newtext = ev->summary();
      if ( ev->isMultiDay() &&  !ev->doesFloat() ) {
        newtext.append( QString(" (%1/%2)").arg( dayof ).arg( span ) );
      }

      KURLLabel *urlLabel = new KURLLabel( this );
      urlLabel->setText( newtext );
      urlLabel->setURL( ev->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setAlignment( urlLabel->alignment() | Qt::WordBreak );
      mLayout->addWidget( urlLabel, counter, 2 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( viewEvent( const QString& ) ) );
      connect( urlLabel, SIGNAL( rightClickedURL( const QString& ) ),
               this, SLOT( popupMenu( const QString& ) ) );

      QString tipText( KCal::IncidenceFormatter::toolTipString( ev, true ) );
      if ( !tipText.isEmpty() ) {
        QToolTip::add( urlLabel, tipText );
      }

      // Fill Event Time Range Field (only for non-floating Events)
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
        datestr = i18n( "Time from - to", "%1 - %2" )
                  .arg( KGlobal::locale()->formatTime( sST ) )
                  .arg( KGlobal::locale()->formatTime( sET ) );
        label = new QLabel( datestr, this );
        label->setAlignment( AlignLeft | AlignVCenter );
        mLayout->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      counter++;
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18n( "No appointments pending within the next day",
            "No appointments pending within the next %n days",
            days ), this, "nothing to see" );
    noEvents->setAlignment( AlignHCenter | AlignVCenter );
    mLayout->addWidget( noEvents, 0, 2 );
    mLabels.append( noEvents );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void SummaryWidget::viewEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

void SummaryWidget::removeEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.deleteIncidence( uid, false );
}

void SummaryWidget::popupMenu( const QString &uid )
{
  KPopupMenu popup( this );
  QToolTip::remove( this );
  popup.insertItem( i18n( "&Edit Appointment..." ), 0 );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "editdelete", KIcon::Small),
                    i18n( "&Delete Appointment" ), 1 );

  switch ( popup.exec( QCursor::pos() ) ) {
    case 0:
      viewEvent( uid );
      break;
    case 1:
      removeEvent( uid );
      break;
  }
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Edit Appointment: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkorgsummary.desktop" );
}

#include "summarywidget.moc"
