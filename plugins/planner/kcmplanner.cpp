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
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

#include <QTabWidget>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

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
  QWidget *widget = new QWidget( parent );
  initGUI();

  customDaysChanged( 1 );

  connect( mCalendarGroup, SIGNAL(clicked(int)), SLOT(modified()) );
  connect( mCalendarGroup, SIGNAL(clicked(int)), SLOT(buttonClicked(int)) );
  connect( mTodoGroup, SIGNAL(clicked(int)), SLOT(modified()) );
  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(modified()) );
  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(customDaysChanged(int)) );

  KAcceleratorManager::manage( this );

  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kcmplanner" ), 0,
                                      ki18n( "Schedule Configuration Dialog" ),
                                      0, KLocalizedString(), KAboutData::License_GPL,
                                      ki18n( "(c) 2007-2008 Oral Timocin" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
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

void KCMPlanner::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 4 );
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
  Configuration Pages
*/

void KCMPlanner::initCalendarPage()
{
  mCalendarPage = new QWidget( this );

  Q3VBoxLayout *layout = new Q3VBoxLayout( mCalendarPage, KDialog::spacingHint() );

  mCalendarGroup = new Q3ButtonGroup( 0, Qt::Vertical, i18n( "Calendar View" ), mCalendarPage );
  Q3VBoxLayout *boxLayout = new Q3VBoxLayout( mCalendarGroup->layout(), KDialog::spacingHint() );

  QRadioButton *button = new QRadioButton( i18n( "One day" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "Five days" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One week" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One month" ), mCalendarGroup );
  boxLayout->addWidget( button );

  Q3HBoxLayout *hbox = new Q3HBoxLayout( boxLayout, KDialog::spacingHint() );

  button = new QRadioButton( "", mCalendarGroup );
  hbox->addWidget( button );

  mCustomDays = new QSpinBox( 1, 365, 1, mCalendarGroup );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  hbox->addStretch( 1 );

  layout->addWidget( mCalendarGroup );
  layout->addStretch();
}

void KCMPlanner::initTodoPage()
{
  mTodoPage = new QWidget( this );
  Q3VBoxLayout *layout = new Q3VBoxLayout( mTodoPage, KDialog::spacingHint() );

  mTodoGroup = new Q3GroupBox( 8, Qt::Vertical, i18n( "Show To-dos" ), mTodoPage );
  mTodoGroup->setCheckable( true );

  mShowAllTodos = new QCheckBox( i18n( "Show all to-dos" ), mTodoGroup );
  mShowOverdueTodos = new QCheckBox( i18n( "Overdue to-dos" ), mTodoGroup );
  mShowTodayStartingTodos = new QCheckBox( i18n( "Starting to-dos" ), mTodoGroup );
  mShowTodayEndingTodos = new QCheckBox( i18n( "Ending to-dos" ), mTodoGroup );
  mShowTodosInProgress = new QCheckBox( i18n( "To-dos in progress" ), mTodoGroup );
  mShowCompleted = new QCheckBox( i18n( "Completed to-dos" ), mTodoGroup );

  Q3HBoxLayout *hbox = new Q3HBoxLayout( layout, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Priority limit:"), mTodoGroup );
  hbox->addWidget( label );
  mPriority = new QSpinBox( 0, 9, 1, mTodoGroup );
  hbox->addWidget( mPriority );

  hbox->addStretch( 2 );

  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(setTodo(bool)) );
  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowAllTodos, SIGNAL(toggled(bool)), SLOT(disableAll(bool)) );
  connect( mShowTodayEndingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodosInProgress, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodayStartingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowOverdueTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowCompleted, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mPriority, SIGNAL(valueChanged(int)), SLOT(modified()) );

  layout->addWidget( mTodoGroup );
  layout->addStretch();
}

void KCMPlanner::initSdPage()
{
  mSdPage = new QWidget( this );
  Q3VBoxLayout *layout = new Q3VBoxLayout( mSdPage, KDialog::spacingHint() );
  mSdGroup = new Q3GroupBox( 5, Qt::Vertical, i18n( "Show Special Dates" ), mSdPage );
  mSdGroup->setCheckable( true );

  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(setSd(bool)) );

  layout->addWidget( mSdGroup );
  layout->addStretch();
}

/*
  Initialization of Config Gui
*/

void KCMPlanner::initGUI()
{
  Q3VBoxLayout *topLayout = new Q3VBoxLayout( this, 0, KDialog::spacingHint() );
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
 *  Read Config Entries
 */

void KCMPlanner::load()
{
  KConfig config( "plannerrc" );

  //Read Calendar Config
  KConfigGroup group = config.group( "Calendar" );

  int days = group.readEntry( "DaysToShow", 1 );
  if ( days == 1 ) {
    mCalendarGroup->setButton( 0 );
  } else if ( days == 5 ) {
    mCalendarGroup->setButton( 1 );
  } else if ( days == 7 ) {
    mCalendarGroup->setButton( 2 );
  } else if ( days == 31 ) {
    mCalendarGroup->setButton( 3 );
  } else {
    mCalendarGroup->setButton( 4 );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  //Read Todo Config
  group = config.group( "Todo" );
  mTodo = group.readEntry( "Todo", false );
  mTodoGroup->setChecked( mTodo );
  mShowAllTodos->setChecked( group.readEntry( "ShowAllTodos", false ) );
  mShowTodayEndingTodos->setChecked( group.readEntry( "ShowTodayEndingTodos", false ) );
  mShowTodosInProgress->setChecked( group.readEntry( "ShowTodosInProgress", false ) );
  mShowTodayStartingTodos->setChecked( group.readEntry( "ShowTodayStartingTodos", false ) );
  mShowOverdueTodos->setChecked( group.readEntry( "ShowOverdueTodos", false ) );
  mShowCompleted->setChecked( group.readEntry( "ShowCompleted", false ) );
  mPriority->setValue( group.readEntry( "MaxPriority", 0 ) );

  //Read Special Dates Config
  group = config.group( "SpecialDates" );
  mSd = group.readEntry( "SpecialDates", false ) ;

  emit changed( false );
}

/*
 *  Store Configuration
 */

void KCMPlanner::save()
{
  KConfig config( "plannerrc" );
  KConfigGroup group = config.group( "Calendar" );

  int days;
  switch ( mCalendarGroup->selectedId() ) {
  case 0:
    days = 1;
    break;
  case 1:
    days = 5;
    break;
  case 2:
    days = 7;
    break;
  case 3:
    days = 31;
    break;
  case 4:
  default:
    days = mCustomDays->value();
    break;
  }
  group.writeEntry( "DaysToShow", days );

  config.group( "Todo" );
  group.writeEntry( "Todo", mTodo );
  group.writeEntry( "ShowAllTodos", mShowAllTodos->isChecked() );
  group.writeEntry( "ShowTodayEndingTodos", mShowTodayEndingTodos->isChecked() );
  group.writeEntry( "ShowTodosInProgress", mShowTodosInProgress->isChecked() );
  group.writeEntry( "ShowTodayStartingTodos", mShowTodayStartingTodos->isChecked() );
  group.writeEntry( "ShowOverdueTodos", mShowOverdueTodos->isChecked() );
  group.writeEntry( "ShowCompleted", mShowCompleted->isChecked() );
  group.writeEntry( "MaxPriority", mPriority->value() );

  config.group( "SpecialDates" );
  group.writeEntry( "SpecialDates", mSd );

  config.sync();

  emit changed( false );
}

/*
 *  Set Config Defaults
 */

void KCMPlanner::defaults()
{
  mCalendarGroup->setButton( 0 );

  mTodo = true;
  mShowAllTodos->setChecked( false );
  mShowTodayEndingTodos->setChecked( true );
  mShowTodosInProgress->setChecked( true );
  mShowTodayStartingTodos->setChecked( true );
  mShowOverdueTodos->setChecked( true );
  mShowCompleted->setChecked( true );
  mPriority->setValue( 5 );

  mSd = true ;

  emit changed( true );
}

#include "kcmplanner.moc"
