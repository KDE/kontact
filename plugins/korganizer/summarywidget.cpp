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
#include <qtooltip.h>
#include <libkcal/event.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkdepim/kpimprefs.h>

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
  QWidget *header = createHeader( this, icon, i18n( "Appointments" ) );
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

  KIconLoader loader( "korganizer" );

  KConfig config( "kcmkorgsummaryrc" );

  config.setGroup( "Calendar" );
  int days = config.readNumEntry( "DaysToShow", 1 );

  QLabel *label = 0;
  int counter = 0;
  QPixmap pm = loader.loadIcon( "appointment", KIcon::Small );

  QDate dt;
  for ( dt=QDate::currentDate();
        dt<=QDate::currentDate().addDays( days - 1 );
        dt=dt.addDays(1) ) {
    KCal::Event::List events = mCalendar->events( dt, true );
    KCal::Event *ev;
    KCal::Event::List::ConstIterator it;
    for ( it=events.begin(); it!=events.end(); ++it ) {
      ev = *it;

      // Count number of days in multiday event
      int span=1; int dayof=1;
      if ( ev->isMultiDay() ) {
        QDate d = ev->dtStart().date();
        while ( d < ev->dtEnd().date() ) {
          if ( d < dt ) {
            dayof++;
          }
          span++;
          d=d.addDays( 1 );
        }
      }

      // Fill Appointment Pixmap Field
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignTop );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Fill Event Date Field
      bool makeBold = false;
      QString datestr;

      // Modify event date for printing
      QDate sD = QDate::QDate( dt.year(), dt.month(), dt.day() );
      if ( ev->isMultiDay() ) {
        sD.setYMD( dt.year(), dt.month(), dt.day() );
      }
      if ( ( sD.month() == QDate::currentDate().month() ) &&
           ( sD.day()   == QDate::currentDate().day() ) ) {
        datestr = i18n( "Today" );
        makeBold = true;
      } else if ( ( sD.month() == QDate::currentDate().addDays( 1 ).month() ) &&
                  ( sD.day()   == QDate::currentDate().addDays( 1 ).day() ) ) {
        datestr = i18n( "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD );
      }
      label = new QLabel( datestr, this );
      label->setAlignment( AlignLeft | AlignTop );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      // Fill Event Summary Field
      QString newtext = ev->summary();
      if ( ev->isMultiDay() ) {
        newtext.append( QString(" (%1/%2)").arg( dayof ).arg( span ) );
      }

      KURLLabel *urlLabel = new KURLLabel( ev->uid(), newtext, this );
      urlLabel->setAlignment( urlLabel->alignment() | Qt::WordBreak );
      mLayout->addWidget( urlLabel, counter, 2 );
      mLabels.append( urlLabel );

      if ( !ev->description().isEmpty() ) {
        QToolTip::add( urlLabel, ev->description() );
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
        label->setAlignment( AlignLeft | AlignTop );
        mLayout->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( selectEvent( const QString& ) ) );

      counter++;
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel( i18n( "No appointments pending" ), this );
    noEvents->setAlignment( AlignRight | AlignVCenter );
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
