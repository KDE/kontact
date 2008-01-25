/*
  This file is part of Kontact.
  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2006 Oral Timocin <oral.timocin@kdemail.net>

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
//Added by qt3to4:
#include <Q3GridLayout>
#include <QPixmap>
#include <QEvent>
#include <Q3VBoxLayout>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/incidence.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/incidenceformatter.h>
#include <libkdepim/kpimprefs.h>
#include <libkholidays/kholidays.h>

#include "core.h"
#include "plugin.h"
#include "plannerplugin.h"
#include "korganizer/stdcalendar.h"
#include "korganizeriface_stub.h"

#include "planner.h"

Planner::Planner( PlannerPlugin *plugin, QWidget *parent,
                  const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin ), mCalendar( 0 )
{
  Q3VBoxLayout *mainLayout = new Q3VBoxLayout( this, 3, 3 );

  QPixmap icon =
    KIconLoader::global()->loadIcon( "kontact_date",
                                     KIconLoader::Desktop, KIconLoader::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Planner" ) );
  mainLayout->addWidget( header );

  mLayout = new Q3GridLayout( mainLayout, 8, 5, 3 );
  mLayout->setRowStretch( 6, 1 );
  mainLayout->addStretch();

  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ),
           SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           SLOT( updateView() ) );

  // Update Configuration
  configUpdated();
}

Planner::~Planner()
{
}

void Planner::configUpdated()
{
  KConfig config( "plannerrc" );
  config.setGroup( "Calendar" );
  mDays = config.readNumEntry( "DaysToShow", 1 );

  config.setGroup( "Todo" );
  mShowTodos = false;
  mPriority = config.readNumEntry( "MaxPriority" );
  mShowAllTodos = config.readBoolEntry( "ShowAllTodos" );
  mShowTodayEndingTodos = config.readBoolEntry( "ShowTodayEndingTodos" );
  mShowTodosInProgress = config.readBoolEntry( "ShowTodosInProgress" );
  mShowTodayStartingTodos = config.readBoolEntry( "ShowTodayStartingTodos" );
  mShowOverdueTodos = config.readBoolEntry( "ShowOverdueTodos" );
  mShowCompleted = config.readBoolEntry( "ShowCompleted" );

  if( mShowAllTodos || mShowTodayEndingTodos || mShowTodosInProgress ||
       mShowTodayStartingTodos || mShowOverdueTodos || mShowCompleted ||
       mPriority ){
    if( config.readBoolEntry( "Todo" ) ){
      mShowTodos = true;
    }
  }

  config.setGroup( "SpecialDates" );
  mShowSd = config.readBoolEntry( "SpecialDates" );

  updateView();
}

void Planner::initTodoList( const QDate date )
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
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->priority() <= mPriority ) {
        priorityList.append( todo );
      }
    }
    todos = priorityList;
  }
  if( mShowOverdueTodos ){
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->hasDueDate() && !todo->isCompleted() &&
          todo->dtDue().date() < date && date == currentDate ) {
        mTodos.append( todo );
      }
    }
  }
  if( mShowTodayEndingTodos ){
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->hasDueDate() && todo->dtDue().date() == date && !todo->isCompleted() ){
        mTodos.append( todo );
      }
    }
  }
  if( mShowTodayStartingTodos ){
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->hasStartDate() && todo->dtStart().date() == date &&
          !todo->isCompleted() ) {
        mTodos.append( todo );
      }
    }
  }
  if( mShowTodosInProgress ){
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < date &&
       date < todo->dtDue().date() && !todo->isCompleted() &&
          date == currentDate ){
        mTodos.append( todo );
      }
    }
  }
  if( mShowCompleted ){
    for( td = todos.begin() ; td != todos.end(); ++td){
      todo = *td;
      if ( todo->isCompleted() && date == currentDate){
        mTodos.append( todo );
      }
    }
  }
}

int Planner::showTodos(int counter, const QDate date)
{
  KIconLoader loader( "kdepim" );
  initTodoList( date );

  if ( !mTodos.empty() ) {
    KCal::Todo *todo;
    KCal::Todo::List::ConstIterator td = mTodos.begin();

    ++counter;
    for ( ; td != mTodos.end() ; ++td ) {
      todo = *td;
      QString stateText = initStateText ( todo , date );
      QPixmap todoPm = loader.loadIcon( "kontact_todo", KIcon::Small );
      QLabel *label = new QLabel( this );
      label->setPixmap( todoPm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 0 );
      mLabels.append( label );

      QPixmap recur;
      if( todo->isAlarmEnabled() ){
        recur = loader.loadIcon( "recur", KIcon::Small );
      }
      label = new QLabel( this );
      label->setPixmap( recur );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 1 );
      mLabels.append( label );

      QPixmap alarm;
      if( todo->isAlarmEnabled() ){
        alarm = loader.loadIcon( "bell", KIcon::Small );
      }
      label = new QLabel( this );
      label->setPixmap( alarm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mPlannerGrid->addWidget( label, counter, 2 );
      mLabels.append( label );

      QString percent = QString::number( todo->percentComplete() ) + '%';
      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( percent );
      urlLabel->setURL( todo->uid() );
      if( stateText == i18n( "overdue" ) ){
        urlLabel->setText( "<font color = red >" + percent + "</font>" );
      }
      urlLabel->setAlignment( AlignLeft | AlignTop );
      urlLabel->setMaximumWidth( urlLabel->minimumSizeHint().width() );
      mPlannerGrid->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL( rightClickedUrl( const QString& ) ),
                this, SLOT( changePercentage( const QString& ) ) );

      QString string = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        string = todo->relatedTo()->summary() + ":" + todo->summary();
      }

      KUrlLabel *urlLabel2 = new KUrlLabel( this );
      urlLabel2->setText( string );
      urlLabel2->setURL( todo->uid() );
      urlLabel2->installEventFilter( this );
      urlLabel2->setAlignment( AlignLeft | AlignTop );
      if( stateText == i18n( "overdue" ) ){
        urlLabel2->setText( "<font color = red >" + string + "</font>" );
      }
      mPlannerGrid->addWidget( urlLabel2, counter, 4 );
      mLabels.append( urlLabel2 );

      connect( urlLabel2, SIGNAL( leftClickedUrl( const QString& ) ),
                this, SLOT( viewTodo( const QString& ) ) );
      connect( urlLabel2, SIGNAL( rightClickedUrl( const QString& ) ),
                this, SLOT( todoPopupMenu( const QString& ) ) );

      label = new QLabel( stateText, this );
      if( stateText == i18n( "overdue" ) ){
        label->setText( "<font color = red >" + stateText + " </font>" );
      }
      label->setAlignment( AlignLeft | AlignTop );
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

void Planner::initEventList( const QDate date)
{
  mEvents.setAutoDelete( true );
  mEvents.clear();
  mEvents.setAutoDelete( false );

  KCal::Event *ev;
  KCal::Event::List events_orig = mCalendar->events( date );
  KCal::Event::List::ConstIterator it = events_orig.begin();
  QDateTime qdt;

  // prevent implicitely sharing while finding recurring events
  // replacing the QDate with the currentDate
  for ( ; it != events_orig.end(); ++it ) {
    ev = ( *it )->clone();
    if ( ev->recursOn( date ) ) {
      qdt = ev->dtStart();
      qdt.setDate( date );
      ev->setDtStart( qdt );
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

int Planner::showEvents( int counter, const QDate date)
{
  KIconLoader loader( "kdepim" );
  initEventList( date );

  if( !mEvents.empty() ){
    KCal::Event *ev;
    KCal::Event::List::ConstIterator it;
    QDate currentDate = QDate::currentDate();
    QString datestr;
    QDate sD = QDate::QDate( date.year(), date.month(), date.day() );
    QLabel *label;

    ++counter;
    for ( it = mEvents.begin(); it != mEvents.end(); ++it ) {
      ev = *it;

      // Count number of days remaining in multiday event
      int span=1; int dayof=1;
      if ( ev->isMultiDay() ){
        QDate d = ev->dtStart().date();
        if ( d < currentDate ){
          d = currentDate;
        }
        while ( d < ev->dtEnd().date() ){
          if ( d < date ){
            dayof++;
          }
          span++;
          d=d.addDays( 1 );
        }
      }

      // If this date is part of a floating, multiday event, then we
      // only make a print for the first day of the event.
      if ( ev->isMultiDay() && ev->doesFloat() && dayof != 1 ) {
        continue;
      }

      //Show Event icon
      QPixmap re = loader.loadIcon( "appointment", KIcon::Small );
      label = new QLabel( this );
      label->setPixmap( re );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignTop );
      mPlannerGrid->addWidget( label, counter, 0 );
      mLabels.append( label );


      //Show icon if Event recurs
      QPixmap recur;
      if( ev->doesRecur() ){
        recur = loader.loadIcon( "recur", KIcon::Small );
      }
      label = new QLabel( this );
      label->setPixmap( recur );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignTop );
      mPlannerGrid->addWidget( label, counter, 1 );
      mLabels.append( label );

      //Show icon if Alarm is enabled
      QPixmap alarm;
      if( ev->isAlarmEnabled () ){
        alarm = loader.loadIcon( "bell", KIcon::Small );
      }
      label = new QLabel( this );
      label->setPixmap( alarm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignTop );
      mPlannerGrid->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( ev->isMultiDay() && ev->doesFloat() && dayof == 1 && span > 1 ) {
        datestr = KGlobal::locale()->formatDate( ev->dtStart().date() );
        datestr += " -\n " +
                    KGlobal::locale()->formatDate( sD.addDays( span-1 ) );
        label = new QLabel( datestr, this );
        label->setAlignment( AlignLeft | AlignTop );
        label->setPaletteForegroundColor( colorGroup().text() );
        mPlannerGrid->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      // Fill Event Time Range Field (only for non-floating Events)
      if ( !ev->doesFloat() ){
        QTime sST = ev->dtStart().time();
        QTime sET = ev->dtEnd().time();
        if ( ev->isMultiDay() ){
          if ( ev->dtStart().date() < date ){
            sST = QTime::QTime( 0, 0 );
          }
          if ( ev->dtEnd().date() > date ){
            sET = QTime::QTime( 23, 59 );
          }
        }

        datestr = i18n( "Time from - to", "%1 - %2" )
                  .arg( KGlobal::locale()->formatTime( sST ) )
                  .arg( KGlobal::locale()->formatTime( sET ) );
        label = new QLabel( datestr, this );
        label->setAlignment( AlignLeft | AlignTop );
        label->setMaximumWidth( label->minimumSizeHint().width() );
        mPlannerGrid->addWidget( label, counter, 3 );
        mLabels.append( label );
      }

      // Fill Event Summary Field
      QString newtext = ev->summary();
      if ( ev->isMultiDay() &&  !ev->doesFloat() ){
        newtext.append( QString(" (%1/%2)").arg( dayof ).arg( span ) );
      }
      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( newtext );
      urlLabel->setURL( ev->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setAlignment( AlignLeft | AlignTop |
                                Qt::WordBreak );
      mPlannerGrid->addWidget( urlLabel, counter, 4 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL( leftClickedUrl( const QString& ) ),
                this, SLOT( viewEvent( const QString& ) ) );
      connect( urlLabel, SIGNAL( rightClickedUrl( const QString& ) ),
                this, SLOT( eventPopupMenu( const QString& ) ) );

      QString tipText( KCal::IncidenceFormatter::toolTipString( ev, true ) );
      if ( !tipText.isEmpty() ){
        QToolTip::add( urlLabel, tipText );
      }

      counter++;
    }
  }
  return counter;
}

void Planner::initSdList( const QDate date)
{

}

int Planner::showSd(int counter)
{
  return counter;
}

void Planner::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

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
    if ( !mEvents.empty() || (!mTodos.empty() && mShowTodos) ){
      ++counter;
      label = new QLabel( this );
      label->setPaletteBackgroundColor( gray.light() );
      mLayout->addMultiCellWidget( label, counter, counter, 0, 4 );
      mLabels.append( label );

      QPixmap pm = loader.loadIcon( "kontact_date", KIcon::Small );
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignLeft | AlignTop );
      label->setPaletteBackgroundColor( gray.light() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Fill Event Date Field
      bool makeBold = false;
      QString datestr;

      // Modify event date for printing
      QDate sD = QDate::QDate( dt.year(), dt.month(), dt.day() );
      if ( ( sD.month() == currentDate.month() ) &&
           ( sD.day() == currentDate.day() ) ) {
        datestr = i18n( "Today" );
        makeBold = true;
      } else if ( ( sD.month() == currentDate.addDays( 1 ).month() ) &&
                  ( sD.day()   == currentDate.addDays( 1 ).day() ) ){
        datestr = i18n( "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD );
      }

      label = new QLabel( datestr, this );
      label->setAlignment( AlignLeft | AlignTop );
      label->setPaletteBackgroundColor( gray.light() );
      if ( makeBold ){
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      ++counter;
      Q3VBoxLayout *todoLayout = new Q3VBoxLayout( this, 3, 3 );
      mPlannerGrid = new Q3GridLayout ( todoLayout, 7 , 6, 3 );
      mPlannerGrid->setRowStretch( 6, 1 );
      todoLayout->addStretch();
      mLayout->addMultiCellLayout( todoLayout, counter, counter, 0, 4 );

      if( !mTodos.empty() && mShowTodos){
        counter = showTodos( counter , dt );
      }
      if( !mEvents.empty() ){
        counter = showEvents( counter, dt );
      }
    }
  }

  if ( !counter ) {
    QLabel *noEvents = new QLabel(
      i18n( "No appointments pending within the next day",
            "No appointments pending within the next %n days",
            mDays ), this, "nothing to see" );
    noEvents->setAlignment( AlignHCenter | AlignVCenter );
    mLayout->addWidget( noEvents, 0, 2 );
    mLabels.append( noEvents );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() ) {
    label->show();
  }
}

/*
 *      Event Section Begin
 */

void Planner::viewEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

void Planner::removeEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_korganizerplugin" ); //ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.deleteIncidence( uid, false );
}

void Planner::eventPopupMenu( const QString &uid )
{
  KMenu popup( this );
  QToolTip::remove( this );
  popup.insertItem( i18n( "&Edit Appointment..." ), 0 );
  popup.insertItem( KIconLoader::global()->loadIcon( "editdelete", KIcon::Small),
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

bool Planner::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel *label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Edit Appointment: \"%1\"" ).arg( label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

/*
 *      Event Section End
 */

/*
 *      Todo Section Begin
 */

QString Planner::initStateText( const KCal::Todo *todo, const QDate date )
{
  QDate currentDate = QDate::currentDate();
  QString stateText;
  // show uncomplete todos from the last days
  if ( todo->hasDueDate() && !todo->isCompleted() &&
       todo->dtDue().date() < currentDate ) {
    stateText = i18n( "overdue" );
  }

  // show todos which started somewhere in the past and has to be finished in future
  if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < date &&
       date < todo->dtDue().date() ) {
    stateText = i18n( "in progress" );
  }

  // all todos which start today
  if ( todo->hasStartDate() && todo->dtStart().date() == date ) {
    stateText = i18n( "starts today" );
  }

  // all todos which end today
  if ( todo->hasDueDate() && todo->dtDue().date() == date ) {
    stateText = i18n( "ends today" );
  }
  if ( todo->isCompleted() ) {
    stateText = i18n( "completed" );
  }
  return stateText;
}

void Planner::todoPopupMenu( const QString &uid )
{
  KMenu popup( this );
  popup.insertItem( i18n( "&Edit To-do..." ), 0 );
  popup.insertItem( KIconLoader::global()->loadIcon( "editdelete", KIcon::Small),
                    i18n( "&Delete To-do" ), 1 );
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isCompleted() ) {
    popup.insertItem( KIconLoader::global()->loadIcon( "checkedbox", KIcon::Small),
                      i18n( "&Mark To-do Completed" ), 2 );
  }

  switch ( popup.exec( QCursor::pos() ) ) {
  case 0:
    viewTodo( uid );
    break;
  case 1:
    removeTodo( uid );
    break;
  case 2:
      completeTodo( uid );
    break;
  }
}

void Planner::viewTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

void Planner::removeTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.deleteIncidence( uid, false );
}

void Planner::completeTodo( const QString &uid )
{
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isReadOnly() && mCalendar->beginChange( todo ) ) {
    todo->setCompleted( QDateTime::currentDateTime() );
    mCalendar->endChange( todo );
    updateView();
  }
}

void Planner::changePercentage( const QString &uid )
{
  KMenu popup( this );
  popup.insertItem( "0%", 0 );
  popup.insertItem( "10%", 1 );
  popup.insertItem( "20%", 2 );
  popup.insertItem( "30%", 3 );
  popup.insertItem( "40%", 4 );
  popup.insertItem( "50%", 5 );
  popup.insertItem( "60%", 6 );
  popup.insertItem( "70%", 7 );
  popup.insertItem( "80%", 8 );
  popup.insertItem( "90%", 9 );
  popup.insertItem( "100%", 10 );

  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isReadOnly() && mCalendar->beginChange( todo )) {
    switch ( popup.exec( QCursor::pos() ) ) {
      case 0: todo->setPercentComplete( 0 ) ;break;
      case 1: todo->setPercentComplete( 10 ) ;break;
      case 2: todo->setPercentComplete( 20 ) ;break;
      case 3: todo->setPercentComplete( 30 ) ;break;
      case 4: todo->setPercentComplete( 40 ) ;break;
      case 5: todo->setPercentComplete( 50 ) ;break;
      case 6: todo->setPercentComplete( 60 ) ;break;
      case 7: todo->setPercentComplete( 70 ) ;break;
      case 8: todo->setPercentComplete( 80 ) ;break;
      case 9: todo->setPercentComplete( 90 ) ;break;
      case 10: todo->setCompleted( true ) ;break;
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
      emit message( i18n( "Edit To-do: \"%1\"" ).arg( label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

/*
 *      Todo Section End
 */

QStringList Planner::configModules() const
{
  return QStringList( "kcmplanner.desktop" );
}

#include "planner.moc"
