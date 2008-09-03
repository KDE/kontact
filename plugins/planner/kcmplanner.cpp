/*
  This file is part of Kontact.
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "kcmplanner.h"

#include <kdemacros.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kbuttongroup.h>

#include <Qt>
#include <QTabWidget>
#include <QButtonGroup>
#include <qlabel.h>
#include <qlayout.h>
#include <QRadioButton>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

extern "C"
{
  KDE_EXPORT KCModule *create_planner( QWidget *parent, const char * )
  {
    KComponentData inst( "kcmplanner" );
    return new KCMPlanner( inst, parent );
  }
}

KCMPlanner::KCMPlanner( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  initGUI();

  customDaysChanged( 1 );

  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(modified()) );
  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(customDaysChanged(int)) );

  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kcmplanner" ), 0,
                                      ki18n( "Planner Summary Configuration Dialog" ),
                                      0, KLocalizedString(), KAboutData::License_GPL,
                                      ki18n( "(c) 2007-2008 Oral Timocin" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
  about->addAuthor( ki18n( "Oral Timocin" ), KLocalizedString(), "oral.timocin@kdemail.net" );
  setAboutData( about );
}

/*
 *  Set mTodo if To-dos should be shown or not
 */

void KCMPlanner::setTodo( bool state )
{
   mTodo = state;
}
 
void KCMPlanner::setSd( bool state )
{
   mSd = state;
}
 
void KCMPlanner::modified()
{
  emit changed( true );
}

void KCMPlanner::buttonClicked( bool state )
{
  mCustomDays->setEnabled( state );
}

void KCMPlanner::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}
 
 /*
  *  If mShowAllTodos is true diasble all other options
  */
 
void KCMPlanner::disableAll( bool state )
{
  mShowTodayEndingTodos->setDisabled( state );
  mShowTodayEndingTodos->setChecked( false );

  mShowTodosInProgress->setDisabled( state );
  mShowTodosInProgress->setChecked( false );

  mShowTodayStartingTodos->setDisabled( state );
  mShowTodayStartingTodos->setChecked( false );

  mShowOverdueTodos->setDisabled( state );
  mShowOverdueTodos->setChecked( false );

  mShowCompleted->setDisabled( state );
  mShowCompleted->setChecked( false );

  mPriority->setDisabled( state );

  emit changed( true );
}

/*
 * Initialization of Config Gui
 */

void KCMPlanner::initGUI()
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout->addWidget( tabWidget );

  //Build Calendar Page
  initCalendarPage();
  //Build Todo Page
  initTodoPage();
  //Build Special Dates Pages
  initSdPage();

  tabWidget->addTab( mCalendarPage, i18n( "Calendar" ) );
  tabWidget->addTab( mTodoPage, i18n( "To-dos" ) );
  tabWidget->addTab( mSdPage, i18n( "Special Dates" ) );
}

/*
 * Configuration Pages
 */

void KCMPlanner::initCalendarPage()
{
  mCalendarPage = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( mCalendarPage );

  //Add a GroupBox with Radiobuttons to Calendarpage
  mCalendarGroup = new QGroupBox( i18n( "Days to show Selection" ), mCalendarPage );
  QVBoxLayout *groupLayout = new QVBoxLayout( mCalendarGroup );

  mDay = new QRadioButton( i18n( "One day" ) );
  mFiveDays = new QRadioButton( i18n( "Five days" ) );
  mWeek = new QRadioButton( i18n( "One week" ) );
  mMonth = new QRadioButton( i18n( "One month" ) );

  groupLayout->addWidget( mDay );
  groupLayout->addWidget( mFiveDays );
  groupLayout->addWidget( mWeek );
  groupLayout->addWidget( mMonth );

  //Add Spinbox for choosing the days to show in summary 
  QHBoxLayout *hbox = new QHBoxLayout( );
  mCalendarSpin = new QRadioButton( "" );
  hbox->addWidget( mCalendarSpin );

  mCustomDays = new QSpinBox( this );
  mCustomDays->setRange( 1, 365 );
  mCustomDays->setValue( 1 );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  connect( mDay, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mFiveDays, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mWeek, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mMonth, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mCalendarSpin, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mCalendarSpin, SIGNAL(toggled(bool)), SLOT(buttonClicked( bool )) );

  groupLayout->addLayout( hbox );
  mCalendarGroup->setLayout( groupLayout );

  QGroupBox *groupBox = new QGroupBox( mCalendarPage );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox );

  mShowEventRecurrence = new QCheckBox( i18n( "Show event recurrence" ) );
  mShowEventReminder = new QCheckBox( i18n( "Show events with reminder" ) );
  mUnderlineEvent = new QCheckBox( i18n( "underline Event label" ) );

  boxLayout->addWidget( mShowEventRecurrence );
  boxLayout->addWidget( mShowEventReminder );
  boxLayout->addWidget( mUnderlineEvent);

  connect( mShowEventRecurrence, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowEventReminder, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mUnderlineEvent, SIGNAL(toggled(bool)), SLOT(modified()) );

  groupBox->setLayout( boxLayout );

  layout->addWidget( mCalendarGroup );
  layout->addWidget( groupBox );
  layout->addStretch();
}
 
void KCMPlanner::initTodoPage()
{
  mTodoPage = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( mTodoPage );
  layout->setSpacing( KDialog::spacingHint() );

  mTodoGroup = new QGroupBox( i18n( "Show To-dos" ), mTodoPage );
  mTodoGroup->setCheckable( true );
  QVBoxLayout *todoLayout = new QVBoxLayout( mTodoGroup );

  mShowAllTodos = new QCheckBox( i18n( "Show all to-dos" ) );
  todoLayout->addWidget( mShowAllTodos );
  mShowOverdueTodos = new QCheckBox( i18n( "Overdue to-dos" ) );
  todoLayout->addWidget( mShowOverdueTodos );
  mShowTodayStartingTodos = new QCheckBox( i18n( "Starting to-dos" ) );
  todoLayout->addWidget( mShowTodayStartingTodos );
  mShowTodayEndingTodos = new QCheckBox( i18n( "Ending to-dos" ) );
  todoLayout->addWidget( mShowTodayEndingTodos );
  mShowTodosInProgress = new QCheckBox( i18n( "To-dos in progress" ) );
  todoLayout->addWidget( mShowTodosInProgress );
  mShowCompleted = new QCheckBox( i18n( "Completed to-dos" ) );
  todoLayout->addWidget( mShowCompleted );

  QHBoxLayout *hbox = new QHBoxLayout( );
  hbox->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Priority limit:") );
  hbox->addWidget( label );
  mPriority = new QSpinBox( this );
  mPriority->setRange( 0, 9 );
  mPriority->setValue( 1 );
  hbox->addWidget( mPriority );
  hbox->addStretch( 2 );

  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(setTodo(bool)) );
  connect( mShowAllTodos, SIGNAL(toggled(bool)), SLOT(disableAll(bool)) );
  connect( mShowTodayEndingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodosInProgress, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodayStartingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowOverdueTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowCompleted, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mPriority, SIGNAL(valueChanged(int)), SLOT(modified()) );

  todoLayout->addLayout( hbox );
  mTodoGroup->setLayout( todoLayout );

  QGroupBox *groupBox = new QGroupBox( mTodoPage );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox );

  mShowTodoRecurrence = new QCheckBox( i18n( "Show todo recurrence") );
  mShowTodoReminder = new QCheckBox( i18n( "Show todo reminder") );
  mUnderlineTodo = new QCheckBox( i18n( "Underline Todo" ) );

  boxLayout->addWidget( mShowTodoRecurrence );
  boxLayout->addWidget( mShowTodoReminder );
  boxLayout->addWidget( mUnderlineTodo );

  connect( mShowTodoRecurrence, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodoReminder, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mUnderlineTodo, SIGNAL(toggled(bool)), SLOT(modified()) );

  groupBox->setLayout( boxLayout );

  layout->addWidget( mTodoGroup );
  layout->addWidget( groupBox );
  layout->addStretch();
}
 
void KCMPlanner::initSdPage()
{
  mSdPage = new QWidget( this );

  QVBoxLayout *layout = new QVBoxLayout( mSdPage );
  layout->setSpacing( KDialog::spacingHint() );

//   mSdGroup = new QGroupBox( 5, Qt::Vertical, i18n( "Show Special Dates" ), mSdPage );
//   mSdGroup->setCheckable( true );
// 
//   connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
//   connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(setSd(bool)) );
// 
//   layout->addWidget( mSdGroup );
  layout->addStretch();
}

/*
 *  Read Config Entries
 */

void KCMPlanner::load()
{
  KConfig config( "plannerrc" );

  //Read Calendar Config
  KConfigGroup calendar = config.group( "Calendar" );

  //Set the count of Days from config 
  int days = calendar.readEntry( "DaysToShow", 1 );
  if ( days == 1 ) {
    mDay->setChecked( true );
  } else if ( days == 5 ) {
    mFiveDays->setChecked( true );
  } else if ( days == 7 ) {
    mWeek->setChecked( true );
  } else if ( days == 31 ) {
    mMonth->setEnabled( true );
  } else {
    mCalendarSpin->setChecked( true );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  mShowEventRecurrence->setChecked( calendar.readEntry( "ShowEventRecurrence", false ) );
  mShowEventReminder->setChecked( calendar.readEntry( "ShowEventReminder", false ) );
  mUnderlineEvent->setChecked( calendar.readEntry( "underlineEvent", false ) );


  //Read Todo Config
  KConfigGroup todo = config.group( "Todo" );

  //Set Todos to show from config
  mTodoGroup->setChecked( todo.readEntry( "Todo", true ) );
  mShowAllTodos->setChecked( todo.readEntry( "ShowAllTodos", false ) );
  mShowTodayEndingTodos->setChecked( todo.readEntry( "ShowTodayEndingTodos", false ) );
  mShowTodosInProgress->setChecked( todo.readEntry( "ShowTodosInProgress", false ) );
  mShowTodayStartingTodos->setChecked( todo.readEntry( "ShowTodayStartingTodos", false ) );
  mShowOverdueTodos->setChecked( todo.readEntry( "ShowOverdueTodos", false ) );
  mShowCompleted->setChecked( todo.readEntry( "ShowCompleted", false ) );
  mPriority->setValue( todo.readEntry( "MaxPriority", 0 ) );

  mShowTodoRecurrence->setChecked( todo.readEntry( "ShowTodoRecurrence", false ) );
  mShowTodoReminder->setChecked( todo.readEntry( "ShowTodoReminder", false ) );
  mUnderlineTodo->setChecked( todo.readEntry( "underlineTodo", false) );

  //Read Special Dates Config
  KConfigGroup sd = config.group( "SpecialDates" );
  mSd = sd.readEntry( "SpecialDates", false ) ;

  emit changed( false );
}

/*
 *  Store Configuration
 */

void KCMPlanner::save()
{
  KConfig config( "plannerrc" );

/*
 * Canedar Section
 */

  KConfigGroup calendar = config.group( "Calendar" );

  int days ;
  if ( mDay->isChecked() ) {
    days = 1;
  } else if ( mFiveDays->isChecked() ) {
    days = 5;
  } else if ( mWeek->isChecked() ) {
    days = 7;
  } else if ( mMonth->isChecked() ) {
    days = 31;
  } else {
    days = mCustomDays->value();
  }
  calendar.writeEntry( "DaysToShow", days );

  calendar.writeEntry( "ShowEventRecurrence", mShowEventRecurrence->isChecked() );
  calendar.writeEntry( "ShowEventReminder", mShowEventReminder->isChecked() );
  calendar.writeEntry( "underlineEvent", mUnderlineEvent->isChecked() );

/*
 * Todo Section
 */

  KConfigGroup todo = config.group( "Todo" );
  todo.writeEntry( "Todo", mTodo );
  todo.writeEntry( "ShowAllTodos", mShowAllTodos->isChecked() );
  todo.writeEntry( "ShowTodayEndingTodos", mShowTodayEndingTodos->isChecked() );
  todo.writeEntry( "ShowTodosInProgress", mShowTodosInProgress->isChecked() );
  todo.writeEntry( "ShowTodayStartingTodos", mShowTodayStartingTodos->isChecked() );
  todo.writeEntry( "ShowOverdueTodos", mShowOverdueTodos->isChecked() );
  todo.writeEntry( "ShowCompleted", mShowCompleted->isChecked() );
  todo.writeEntry( "MaxPriority", mPriority->value() );

  todo.writeEntry( "ShowTodoRecurrence", mShowTodoRecurrence->isChecked() );
  todo.writeEntry( "ShowTodoReminder", mShowTodoReminder->isChecked() );
  todo.writeEntry( "underlineTodo", mUnderlineTodo->isChecked() );

/*
 * Special Dates Section
 */

  KConfigGroup sd = config.group( "SpecialDates" );
  sd.writeEntry( "SpecialDates", mSd );

  config.sync();

  emit changed( false );
}

/*
 *  Set Config Defaults
 */

 void KCMPlanner::defaults()
{
//   mCalendarGroup->setButton( 0 );
  mShowEventRecurrence->setChecked( false );
  mShowEventReminder->setChecked( false );
  mUnderlineEvent->setChecked( false );


  mTodo = true;
  mShowAllTodos->setChecked( false );
  mShowTodayEndingTodos->setChecked( true );
  mShowTodosInProgress->setChecked( true );
  mShowTodayStartingTodos->setChecked( true );
  mShowOverdueTodos->setChecked( true );
  mShowCompleted->setChecked( true );
  mPriority->setValue( 5 );

  mShowTodoRecurrence->setChecked( false );
  mShowTodoReminder->setChecked( false );
  mUnderlineTodo->setChecked( false );

  mSd = true ;

  emit changed( true );
}

#include "kcmplanner.moc"
