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
#include <libkcal/todo.h>

#include "core.h"
#include "plugin.h"

#include "summarywidget.h"

SummaryWidget::SummaryWidget( Kontact::Plugin*, QWidget *parent,
                              const char *name )
  : Kontact::Summary( parent, name )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );
  QHBoxLayout *hbox = new QHBoxLayout( mainLayout, 3 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( this );
  label->setFixedSize( 32, 32 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "korganizer", 
                    KIcon::Desktop, KIcon::SizeMedium ) );
  hbox->addWidget( label );

  label = new QLabel( i18n( "Appointments" ), this );
  label->setAlignment( AlignLeft );
  label->setFont( boldFont );
  hbox->addWidget( label );

  mLayout = new QGridLayout( mainLayout, 7, 5, 3 );
  mLayout->setRowStretch( 6, 1 );

  KConfig config( "korganizerrc" );
  mCalendar = new KCal::CalendarResources( config.readEntry( "TimeZoneId" ) );
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  if ( manager->isEmpty() ) {
    config.setGroup("General");
    QString fileName = config.readPathEntry( "Active Calendar" );

    QString resourceName;
    if ( fileName.isEmpty() ) {
      fileName = locateLocal( "appdata", "std.ics" );
      resourceName = i18n("Default KOrganizer resource");
    } else {
      resourceName = i18n("Active Calendar");
    }

    KCal::ResourceCalendar *defaultResource = new KCal::ResourceLocal( fileName );
    defaultResource->setResourceName( resourceName );

    manager->add( defaultResource );
    manager->setStandardResource( defaultResource );
  }

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

    KCal::Event::List::ConstIterator it2;
    for( it2 = events.begin(); it2 != events.end() && counter < 5; ++it2 ) {
      KCal::Event *ev = *it2;
      if ( !ev->recurrence()->doesRecur() || ev->recursOn( QDate::currentDate() ) ) {
        if ( !ev->doesFloat() ) {
          label = new QLabel( this );
          label->setPixmap( pm );
          mLayout->addWidget( label, counter, 0 );
          mLabels.append( label );

          QString date = ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
          label = new QLabel( date, this );
          mLayout->addWidget( label, counter, 1 );
          mLabels.append( label );

          KURLLabel *urlLabel = new KURLLabel( ev->uid(), ev->summary(), this );
          mLayout->addWidget( urlLabel, counter, 2 );
          mLabels.append( urlLabel );
          
          connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
                   this, SLOT( selectEvent( const QString& ) ) );

          counter++;
        }
      }
    }
  }

  KCal::Todo::List todos = mCalendar->todos();
  if ( todos.count() > 0 ) {
    QPixmap pm = loader.loadIcon( "todo", KIcon::Small );
    KCal::Todo::List::ConstIterator it;
    for( it = todos.begin(); it != todos.end(); ++it ) {
      KCal::Todo *todo = *it;
      if ( todo->hasDueDate() && todo->dtDue().date() == QDate::currentDate() ) {
        label = new QLabel( this );
        label->setPixmap( pm );
        mLayout->addWidget( label, counter, 0 );
        mLabels.append( label );

        KURLLabel *urlLabel = new KURLLabel( todo->uid(), todo->summary(), this );
        mLayout->addWidget( urlLabel, counter, 2 );
        mLabels.append( urlLabel );
          
        connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
                 this, SLOT( selectEvent( const QString& ) ) );

        counter++;
      }
    }
  }

  show();
}

void SummaryWidget::selectEvent( const QString & )
{
/*
  DCOPRef dcopCall( mDCOPApp.latin1(), "KOrganizerIface" );
  dcopCall.send( "showEventEditor(QString)", uid );
*/
}

#include "summarywidget.moc"
