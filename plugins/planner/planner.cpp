/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2006 Oral Timocin <oral.timocin@kdemail.net>
  Copyright (c) 2008 Allen Winter <winter@kde.org>

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

#include "planner.h"
#include "plannerplugin.h"
#include "korganizerinterface.h"

#include <korganizer/stdcalendar.h>
#include <kontactinterfaces/core.h>
#include <kontactinterfaces/plugin.h>
#include <libkdepim/kpimprefs.h>
#include <libkholidays/kholidays.h>

#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/incidence.h>
#include <kcal/resourcecalendar.h>
#include <kcal/resourcelocal.h>
#include <kcal/incidenceformatter.h>

#include <kdialog.h>
#include <kdatetime.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kparts/part.h>

#include <QCursor>
#include <QGridLayout>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QToolTip>
#include <QVBoxLayout>

Planner::Planner( Kontact::Plugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ), mPlugin( plugin ), mCalendar( 0 )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );

  //TODO we want our own icon
  QWidget *header = createHeader( this, "view-calendar-tasks", i18n( "Planner" ) );
  mainLayout->addWidget( header );
  mainLayout->addStretch();

  mLayout = new QGridLayout( mainLayout, 8, 5, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL(calendarChanged()), SLOT(updateView()) );
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)), SLOT(updateView()) );

  // Update Configuration
  configUpdated();
}

Planner::~Planner()
{
}

void Planner::configUpdated()
{
  KConfig config( "plannerrc" );

  KConfigGroup group = config.group( "Calendar" );
  mDays = group.readEntry( "DaysToShow", 1 );

  group = config.group( "Todo" );
  mShowTodos = false;
  mPriority = group.readEntry( "MaxPriority", 0 );
  mShowAllTodos = group.readEntry( "ShowAllTodos", false );
  mShowTodayEndingTodos = group.readEntry( "ShowTodayEndingTodos", false );
  mShowTodosInProgress = group.readEntry( "ShowTodosInProgress", false );
  mShowTodayStartingTodos = group.readEntry( "ShowTodayStartingTodos", false );
  mShowOverdueTodos = group.readEntry( "ShowOverdueTodos", false );
  mShowCompleted = group.readEntry( "ShowCompleted", false );

  if ( mShowAllTodos || mShowTodayEndingTodos || mShowTodosInProgress ||
       mShowTodayStartingTodos || mShowOverdueTodos || mShowCompleted ||
       mPriority ){
    if ( group.readEntry( "Todo", false ) ){
      mShowTodos = true;
    }
  }

  group = config.group( "SpecialDates" );
  mShowSd = group.readEntry( "SpecialDates", false );

  updateView();
}

void Planner::initTodoList( const QDate &date )
{
  mTodos.setAutoDelete( true );
  mTodos.clear();
  mTodos.setAutoDelete( false );
  QDate currentDate = QDate::currentDate();
  KCal::Todo *todo;
  KCal::Todo::List todos = mCalendar->todos();
  KCal::Todo::List::ConstIterator td;

  if( mShowAllTodos && date == currentDate ){
    mTodos = mCalendar->todos();
  }
  if( mPriority > 0 ){
    KCal::Todo::List priorityList;
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->priority() <= mPriority ) {
        priorityList.append( todo );
      }
    }
    todos = priorityList;
  }
  if ( mShowOverdueTodos ) {
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->hasDueDate() && !todo->isCompleted() &&
           todo->dtDue().date() < date && date == currentDate ) {
        mTodos.append( todo );
      }
    }
  }
  if ( mShowTodayEndingTodos ) {
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->hasDueDate() && todo->dtDue().date() == date && !todo->isCompleted() ){
        mTodos.append( todo );
      }
    }
  }
  if ( mShowTodayStartingTodos ) {
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->hasStartDate() && todo->dtStart().date() == date && !todo->isCompleted() ) {
        mTodos.append( todo );
      }
    }
  }
  if ( mShowTodosInProgress ) {
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < date &&
       date < todo->dtDue().date() && !todo->isCompleted() &&
          date == currentDate ){
        mTodos.append( todo );
      }
    }
  }
  if ( mShowCompleted ) {
    for ( td = todos.begin(); td != todos.end(); ++td ) {
      todo = *td;
      if ( todo->isCompleted() && date == currentDate ) {
        mTodos.append( todo );
      }
    }
  }
}

int Planner::showTodos( int counter, const QDate &date )
{
  KIconLoader loader( "kdepim" );
  initTodoList( date );

  if ( !mTodos.empty() ) {
    KCal::Todo *todo;
    KCal::Todo::List::ConstIterator td = mTodos.begin();

    ++counter;
    for ( ; td != mTodos.end() ; ++td ) {
      todo = *td;
      QString stateText = initStateText ( todo, date );
      QPixmap todoPm = loader.loadIcon( "view-pim-tasks", KIconLoader::Small );
      QLabel *label = new QLabel( this );
      label->setPixmap( todoPm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 0 );
      mLabels.append( label );

      QPixmap recur;
      if( todo->isAlarmEnabled() ){
        recur = loader.loadIcon( "task-recurring", KIconLoader::Small );
      }
      label = new QLabel( this );
      label->setPixmap( recur );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 1 );
      mLabels.append( label );

      QPixmap alarm;
      if( todo->isAlarmEnabled() ){
        alarm = loader.loadIcon( "task-reminder", KIconLoader::Small );
      }
      label = new QLabel( this );
      label->setPixmap( alarm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 2 );
      mLabels.append( label );

      QString percent = QString::number( todo->percentComplete() ) + '%';
      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( percent );
      urlLabel->setUrl( todo->uid() );
      if( stateText == i18nc( "to-do is overdue", "overdue" ) ){
        urlLabel->setText( "<font color = red >" + percent + "</font>" );
      }
      urlLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop );
      urlLabel->setWordWrap( true );
      urlLabel->setMaximumWidth( urlLabel->minimumSizeHint().width() );
      mPlannerGrid->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(changePercentage(const QString&)) );

      QString string = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        string = todo->relatedTo()->summary() + ':' + todo->summary();
      }

      KUrlLabel *urlLabel2 = new KUrlLabel( this );
      urlLabel2->setText( string );
      urlLabel2->setUrl( todo->uid() );
      urlLabel2->installEventFilter( this );
      urlLabel2->setAlignment( Qt::AlignLeft | Qt::AlignTop );
      urlLabel->setWordWrap( true );
      if( stateText == i18nc( "to-do is overdue", "overdue" ) ){
        urlLabel2->setText( "<font color = red >" + string + "</font>" );
      }
      mPlannerGrid->addWidget( urlLabel2, counter, 4 );
      mLabels.append( urlLabel2 );

      connect( urlLabel2, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(viewTodo(const QString&)) );
      connect( urlLabel2, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(todoPopupMenu(const QString&)) );

      label = new QLabel( stateText, this );
      if ( stateText == i18nc( "to-do is overdue", "overdue" ) ) {
        label->setText( "<font color = red >" + stateText + " </font>" );
      }
      label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 6 );
      mLabels.append( label );

      if ( td != mTodos.end() ) {
        ++counter;
      }
    }
  }
  return counter;
}

void Planner::initEventList( const QDate &date )
{
  mEvents.setAutoDelete( true );
  mEvents.clear();
  mEvents.setAutoDelete( false );

  KCal::Event *ev;
  KCal::Event::List events_orig = mCalendar->events( date );
  KCal::Event::List::ConstIterator it = events_orig.begin();
  KDateTime kdt;

  // prevent implicitely sharing while finding recurring events
  // replacing the QDate with the currentDate
  for ( ; it != events_orig.end(); ++it ) {
    ev = ( *it )->clone();
    if ( ev->recursOn( date, KDateTime::LocalZone ) ) {
      kdt = ev->dtStart();
      kdt.setDate( date );
      ev->setDtStart( kdt );
    }
    mEvents.append( ev );
  }

  // sort the events for this date by summary
  mEvents = KCal::Calendar::sortEvents( &mEvents, KCal::EventSortSummary,
                                        KCal::SortDirectionAscending );
  // sort the events for this date by start date
  mEvents = KCal::Calendar::sortEvents( &mEvents, KCal::EventSortStartDate,
                                        KCal::SortDirectionAscending );
}

int Planner::showEvents( int counter, const QDate &date )
{
  KIconLoader loader( "kdepim" );
  initEventList( date );

  if ( !mEvents.empty() ) {
    KCal::Event *ev;
    KCal::Event::List::ConstIterator it;
    QDate currentDate = QDate::currentDate();
    QString datestr;
    QDate sD = QDate( date.year(), date.month(), date.day() );
    QLabel *label;

    ++counter;
    for ( it = mEvents.begin(); it != mEvents.end(); ++it ) {
      ev = *it;

      // Count number of days remaining in multiday event
      int span = 1;
      int dayof = 1;
      if ( ev->isMultiDay() ) {
        QDate d = ev->dtStart().date();
        if ( d < currentDate ) {
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

      // If this date is part of an allDay, multiday event, then we
      // only make a print for the first day of the event.
      if ( ev->isMultiDay() && ev->allDay() && dayof != 1 ) {
        continue;
      }

      //Show Event icon
      QPixmap re = loader.loadIcon( "view-calendar-day", KIconLoader::Small );
      label = new QLabel( this );
      label->setPixmap( re );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignTop );
      mPlannerGrid->addWidget( label, counter, 0 );
      mLabels.append( label );

      //Show icon if Event recurs
      QPixmap recur;
      if( ev->recurs() ){
        recur = loader.loadIcon( "appointment-recurring", KIconLoader::Small );
      }
      label = new QLabel( this );
      label->setPixmap( recur );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignTop );
      mPlannerGrid->addWidget( label, counter, 1 );
      mLabels.append( label );

      //Show icon if Alarm is enabled
      QPixmap alarm;
      if( ev->isAlarmEnabled () ){
        alarm = loader.loadIcon( "task-reminder", KIconLoader::Small );
      }
      label = new QLabel( this );
      label->setPixmap( alarm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignTop );
      mPlannerGrid->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Print the date span for multiday, allDay events, for the
      // first day of the event only.
      if ( ev->isMultiDay() && ev->allDay() && dayof == 1 && span > 1 ) {
        datestr = KGlobal::locale()->formatDate( ev->dtStart().date() );
        datestr += " -\n " +
                    KGlobal::locale()->formatDate( sD.addDays( span-1 ) );
        label = new QLabel( datestr, this );
        label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        mPlannerGrid->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      // Fill Event Time Range Field (only for non-allDay Events)
      if ( !ev->allDay() ){
        QTime sST = ev->dtStart().time();
        QTime sET = ev->dtEnd().time();
        if ( ev->isMultiDay() ){
          if ( ev->dtStart().date() < date ){
            sST = QTime( 0, 0 );
          }
          if ( ev->dtEnd().date() > date ){
            sET = QTime( 23, 59 );
          }
        }

        datestr = i18nc( "Time from - to", "%1 - %2",
                         KGlobal::locale()->formatTime( sST ),
                         KGlobal::locale()->formatTime( sET ) );
        label = new QLabel( datestr, this );
        label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        label->setMaximumWidth( label->minimumSizeHint().width() );
        mPlannerGrid->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      // Fill Event Summary Field
      QString newtext = ev->summary();
      if ( ev->isMultiDay() &&  !ev->allDay() ){
        newtext.append( QString( " (%1/%2)" ).arg( dayof ).arg( span ) );
      }
      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( newtext );
      urlLabel->setUrl( ev->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop | Qt::WordBreak );
      urlLabel->setWordWrap( true );
      mPlannerGrid->addWidget( urlLabel, counter, 4 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(viewEvent(const QString&)) );
      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(eventPopupMenu(const QString&)) );

      QString tipText( KCal::IncidenceFormatter::toolTipString( ev, true ) );
      if ( !tipText.isEmpty() ) {
        QToolTip::add( urlLabel, tipText );
      }

      counter++;
    }
  }
  return counter;
}

void Planner::initSdList( const QDate &date )
{
}

int Planner::showSd( int counter )
{
  return counter;
}

void Planner::updateView()
{
  while ( !mLabels.isEmpty() ) {
    delete mLabels.takeFirst();
  }

  KIconLoader loader( "kdepim" );

  QLabel *label = 0;
  int counter = 0;

  QDate dt;
  QDate currentDate = QDate::currentDate();

  for ( dt = currentDate;
        dt <= currentDate.addDays( mDays - 1 );
        dt = dt.addDays( 1 ) ) {

    //Initialize Todo List
    initTodoList( dt );
    //Initialize Event List
    initEventList( dt );

    // Fill Appointment Pixmap Field
    if ( !mEvents.empty() || ( !mTodos.empty() && mShowTodos ) ) {
      ++counter;
      label = new QLabel( this );
      label->setPaletteBackgroundColor( Qt::lightGray );
      mLayout->addMultiCellWidget( label, counter, counter, 0, 4 );
      mLabels.append( label );

      QPixmap pm = loader.loadIcon( "view-pim-calendar", KIconLoader::Small );
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
      label->setPaletteBackgroundColor( Qt::lightGray );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Fill Event Date Field
      bool makeBold = false;
      QString datestr;

      // Modify event date for printing
      QDate sD = QDate( dt.year(), dt.month(), dt.day() );
      if ( ( sD.month() == currentDate.month() ) &&
           ( sD.day() == currentDate.day() ) ) {
        datestr = i18nc( "today", "Today" );
        makeBold = true;
      } else if ( ( sD.month() == currentDate.addDays( 1 ).month() ) &&
                  ( sD.day()   == currentDate.addDays( 1 ).day() ) ) {
        datestr = i18nc( "tomorrow", "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD );
      }

      label = new QLabel( datestr, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignTop );
      label->setBackgroundRole( QPalette::Midlight );
      if ( makeBold ){
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      ++counter;
      QVBoxLayout *todoLayout = new QVBoxLayout( this );
      mPlannerGrid = new QGridLayout ( todoLayout, 7, 6, 3 );
      mPlannerGrid->setRowStretch( 6, 1 );
      todoLayout->addStretch();
      mLayout->addMultiCellLayout( todoLayout, counter, counter, 0, 4 );

      if ( !mTodos.empty() && mShowTodos ) {
        counter = showTodos( counter, dt );
      }
      if ( !mEvents.empty() ) {
        counter = showEvents( counter, dt );
      }
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18np( "No appointments pending within the next day",
             "No appointments pending within the next %1 days", mDays ),
      this, "nothing to see" );
    noEvents->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noEvents, 0, 2 );
    mLabels.append( noEvents );
  }

  QListIterator<QLabel *> i( mLabels );
  QLabel *l;
  while ( i.hasNext() ) {
    l = i.next();
    l->show();
  }
}

void Planner::viewEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" );
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.editIncidence( uid );
}

void Planner::removeEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" );
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.deleteIncidence( uid, false );
}

void Planner::eventPopupMenu( const QString &uid )
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

bool Planner::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel *label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Edit Appointment: \"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString() );
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QString Planner::initStateText( const KCal::Todo *todo, const QDate &date )
{
  QDate currentDate = QDate::currentDate();
  QString stateText;
  // show uncomplete todos from the last days
  if ( todo->hasDueDate() && !todo->isCompleted() &&
       todo->dtDue().date() < currentDate ) {
    stateText = i18nc( "to-do is overdue", "overdue" );
  }

  // show todos which started somewhere in the past and has to be finished in future
  if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < date &&
       date < todo->dtDue().date() ) {
    stateText = i18nc( "work on to-do is in progress", "in progress" );
  }

  // all todos which start today
  if ( todo->hasStartDate() && todo->dtStart().date() == date ) {
    stateText = i18nc( "to-do starts today", "starts today" );
  }

  // all todos which end today
  if ( todo->hasDueDate() && todo->dtDue().date() == date ) {
    stateText = i18nc( "to-do ends today", "ends today" );
  }
  if ( todo->isCompleted() ) {
    stateText = i18nc( "to-do is completed", "completed" );
  }
  return stateText;
}

void Planner::todoPopupMenu( const QString &uid )
{
  KMenu popup( this );
  QAction *editIt = popup.addAction( i18n( "&Edit To-do..." ) );
  QAction *delIt = popup.addAction( i18n( "&Delete To-do" ) );
  delIt->setIcon( KIconLoader::global()->
                  loadIcon( "edit-delete", KIconLoader::Small ) );
  QAction *doneIt = 0;
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isCompleted() ) {
    doneIt = popup.addAction( i18n( "&Mark To-do Completed" ) );
    doneIt->setIcon( KIconLoader::global()->
    loadIcon( "task-complete", KIconLoader::Small ) );
  }
  // TODO: add icons to the menu actions

  const QAction *selectedAction = popup.exec( QCursor::pos() );
  if ( selectedAction == editIt ) {
    viewTodo( uid );
  } else if ( selectedAction == delIt ) {
    removeTodo( uid );
  } else if ( doneIt && selectedAction == doneIt ) {
    completeTodo( uid );
  }
}

void Planner::viewTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.editIncidence( uid );
}

void Planner::removeTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.deleteIncidence( uid, false );
}

void Planner::completeTodo( const QString &uid )
{
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isReadOnly() && mCalendar->beginChange( todo ) ) {
    todo->setCompleted( KDateTime::currentLocalDateTime() );
    mCalendar->endChange( todo );
    updateView();
  }
}

void Planner::changePercentage( const QString &uid )
{
  KMenu popup( this );
  QAction *per00 = popup.addAction( i18n( "0%" ) );
  QAction *per10 = popup.addAction( i18n( "10%" ) );
  QAction *per20 = popup.addAction( i18n( "20%" ) );
  QAction *per30 = popup.addAction( i18n( "30%" ) );
  QAction *per40 = popup.addAction( i18n( "40%" ) );
  QAction *per50 = popup.addAction( i18n( "50%" ) );
  QAction *per60 = popup.addAction( i18n( "60%" ) );
  QAction *per70 = popup.addAction( i18n( "70%" ) );
  QAction *per80 = popup.addAction( i18n( "80%" ) );
  QAction *per90 = popup.addAction( i18n( "90%" ) );
  QAction *per100 = popup.addAction( i18n( "100%" ) );

  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isReadOnly() && mCalendar->beginChange( todo ) ) {
    const QAction *selectedAction = popup.exec( QCursor::pos() );
    if ( selectedAction == per00 ) {
      todo->setPercentComplete( 0 );
    } else if ( selectedAction == per10 ) {
      todo->setPercentComplete( 10 );
    } else if ( selectedAction == per20 ) {
      todo->setPercentComplete( 20 );
    } else if ( selectedAction == per30 ) {
      todo->setPercentComplete( 30 );
    } else if ( selectedAction == per40 ) {
      todo->setPercentComplete( 40 );
    } else if ( selectedAction == per50 ) {
      todo->setPercentComplete( 50 );
    } else if ( selectedAction == per60 ) {
      todo->setPercentComplete( 60 );
    } else if ( selectedAction == per70 ) {
      todo->setPercentComplete( 70 );
    } else if ( selectedAction == per80 ) {
      todo->setPercentComplete( 80 );
    } else if ( selectedAction == per90 ) {
      todo->setPercentComplete( 90 );
    } else if ( selectedAction == per100 ) {
      todo->setCompleted( true );
    }
    mCalendar->endChange( todo );
    updateView();
  }
}

bool Planner::todoEventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel *label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Edit To-do: \"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString() );
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList Planner::configModules() const
{
  return QStringList( "kcmplanner.desktop" );
}

#include "planner.moc"
