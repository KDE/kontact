/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>

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
#include "summaryeventinfo.h"
#include "korganizerplugin.h"
#include "korganizerinterface.h"

#include <korganizer/stdcalendar.h>
#include <kontactinterfaces/core.h>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kurllabel.h>

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
  mCalendar->load();

  connect( mCalendar, SIGNAL(calendarChanged()), this, SLOT(updateView()) );
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)), this, SLOT(updateView()) );

  // Update Configuration
  configUpdated();
}

ApptSummaryWidget::~ApptSummaryWidget()
{
}

void ApptSummaryWidget::configUpdated()
{
  KConfig config( "kcmapptsummaryrc" );

  KConfigGroup group = config.group( "Days" );
  mDaysAhead = group.readEntry( "DaysToShow", 7 );

  group = config.group( "Show" );
  mShowBirthdaysFromCal = group.readEntry( "BirthdaysFromCalendar", true );
  mShowAnniversariesFromCal = group.readEntry( "AnniversariesFromCalendar", true );

  updateView();
}

void ApptSummaryWidget::updateView()
{
  qDeleteAll( mLabels );
  mLabels.clear();

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

  SummaryEventInfo::setShowSpecialEvents( mShowBirthdaysFromCal,
                                          mShowAnniversariesFromCal );
  QDate currentDate = QDate::currentDate();
  QDate dt;
  for ( dt = currentDate;
        dt <= currentDate.addDays( mDaysAhead - 1 );
        dt = dt.addDays( 1 ) ) {

    SummaryEventInfo::List events =
        SummaryEventInfo::eventsForDate( dt, mCalendar );

    foreach ( SummaryEventInfo *event, events ) {

      // Icon label
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Start date or date span label
      QString dateToDisplay = event->startDate;
      if ( !event->dateSpan.isEmpty() ) {
        dateToDisplay = event->dateSpan;
      }
      label = new QLabel( dateToDisplay, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );
      if ( event->makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      // Days to go label
      label = new QLabel( event->daysToGo, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Summary label
      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( event->summaryText );
      urlLabel->setUrl( event->summaryUrl );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      urlLabel->setWordWrap( true );
      mLayout->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(viewEvent(const QString&)) );
      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(popupMenu(const QString&)) );
      if ( !event->summaryTooltip.isEmpty() ) {
        urlLabel->setToolTip( event->summaryTooltip );
      }

      // Time range label (only for non-floating events)
      if ( !event->timeRange.isEmpty() ) {
        label = new QLabel( event->timeRange, this );
        label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        mLayout->addWidget( label, counter, 4 );
        mLabels.append( label );
      }

      counter++;
    }

    qDeleteAll( events );
    events.clear();
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18np( "No upcoming events starting within the next day",
             "No upcoming events starting within the next %1 days",
             mDaysAhead ), this );
    noEvents->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noEvents, 0, 0 );
    mLabels.append( noEvents );
  }

  Q_FOREACH( label, mLabels ) { //krazy:exclude=foreach as label is a pointer
    label->show();
  }
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

QStringList ApptSummaryWidget::configModules() const
{
  return QStringList( "kcmapptsummary.desktop" );
}

#include "apptsummarywidget.moc"
