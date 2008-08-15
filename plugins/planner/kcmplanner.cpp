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

#include <Qt>
#include <QTabWidget>
#include <QButtonGroup>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
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
  //QWidget *widget = new QWidget( parent );
  initGUI();

  customDaysChanged( 1 );

//   connect( mCalendarGroup, SIGNAL(clicked(int)), SLOT(modified()) );
//   connect( mCalendarGroup, SIGNAL(clicked(int)), SLOT(buttonClicked(int)) );
//   connect( mTodoGroup, SIGNAL(clicked(int)), SLOT(modified()) );
   connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(modified()) );
   connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(customDaysChanged(int)) );

  KAcceleratorManager::manage( this );

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

  QRadioButton *button = new QRadioButton( i18n( "One day" ) );
  layout->addWidget( button );
  button = new QRadioButton( i18n( "Five days" ) );
  layout->addWidget( button );
  button = new QRadioButton( i18n( "One week" ) );
  layout->addWidget( button );
  button = new QRadioButton( i18n( "One month" ) );
  layout->addWidget( button );

  QHBoxLayout *hbox = new QHBoxLayout( layout );
  hbox->setSpacing( KDialog::spacingHint() );

  button = new QRadioButton( "" );
  
  hbox->addWidget( button );

  mCustomDays = new QSpinBox( this );
  mCustomDays->setRange( 1, 365 );
  mCustomDays->setValue( 1 );
  mCustomDays->setEnabled( false );
  connect( button, SIGNAL(clicked()), SLOT(mCustomDays->setEnabled( true )) );
  hbox->addWidget( mCustomDays );

  hbox->addStretch( 3 );

  layout->addStretch();

}
 
void KCMPlanner::initTodoPage()
{
  mTodoPage = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( mTodoPage );
  layout->setSpacing( KDialog::spacingHint() );


//   mTodoGroup = new QGroupBox( 8, Qt::Vertical, i18n( "Show To-dos" ), mTodoPage );
//   mTodoGroup = new QGroupBox( i18n( "Show To-dos" ), mTodoPage);
//   mTodoGroup->setCheckable( true );
//   mTodoGroup->setLayoutDirection( QT::Vertical );

  mShowAllTodos = new QCheckBox( i18n( "Show all to-dos" ) );
  layout->addWidget( mShowAllTodos );
  mShowOverdueTodos = new QCheckBox( i18n( "Overdue to-dos" ) );
  layout->addWidget( mShowOverdueTodos );
  mShowTodayStartingTodos = new QCheckBox( i18n( "Starting to-dos" ) );
  layout->addWidget( mShowTodayStartingTodos );
  mShowTodayEndingTodos = new QCheckBox( i18n( "Ending to-dos" ) );
  layout->addWidget( mShowTodayEndingTodos );
  mShowTodosInProgress = new QCheckBox( i18n( "To-dos in progress" ) );
  layout->addWidget( mShowTodosInProgress );
  mShowCompleted = new QCheckBox( i18n( "Completed to-dos" ) );
  layout->addWidget( mShowCompleted );

  QHBoxLayout *hbox = new QHBoxLayout( layout );
  hbox->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Priority limit:") );
  hbox->addWidget( label );
  mPriority = new QSpinBox( this );
  mPriority->setRange( 0, 9 );
  mPriority->setValue( 1 );
  hbox->addWidget( mPriority );

  hbox->addStretch( 2 );

//   connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(setTodo(bool)) );
//   connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowAllTodos, SIGNAL(toggled(bool)), SLOT(disableAll(bool)) );
  connect( mShowTodayEndingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodosInProgress, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodayStartingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowOverdueTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowCompleted, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mPriority, SIGNAL(valueChanged(int)), SLOT(modified()) );
 
//   layout->addWidget( mTodoGroup );
  layout->addStretch();
}
 
void KCMPlanner::initSdPage()
{
  mSdPage = new QWidget( this );

  QVBoxLayout *layout = new QVBoxLayout( mSdPage );
  layout->setSpacing( KDialog::spacingHint() );

/*  mSdGroup = new QGroupBox( 5, Qt::Vertical, i18n( "Show Special Dates" ), mSdPage );
  mSdGroup->setCheckable( true );

  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(setSd(bool)) );

  layout->addWidget( mSdGroup );*/
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

  int days = calendar.readEntry( "DaysToShow", 1 );
//   if ( days == 1 ) {
//     mCalendarGroup->setButton( 0 );
//   } else if ( days == 5 ) {
//     mCalendarGroup->setButton( 1 );
//   } else if ( days == 7 ) {
//     mCalendarGroup->setButton( 2 );
//   } else if ( days == 31 ) {
//     mCalendarGroup->setButton( 3 );
//   } else {
//     mCalendarGroup->setButton( 4 );
//     mCustomDays->setValue( days );
//     mCustomDays->setEnabled( true );
//   }

  //Read Todo Config
  KConfigGroup todo = config.group( "Todo" );
  if ( todo.hasKey( "Todo" ) ) {
    mTodo = todo.readEntry( "Todo", false );
    //mTodoGroup->setChecked( mTodo );
  }
  
  mShowAllTodos->setChecked( todo.readEntry( "ShowAllTodos", false ) );
  mShowTodayEndingTodos->setChecked( todo.readEntry( "ShowTodayEndingTodos", false ) );
  mShowTodosInProgress->setChecked( todo.readEntry( "ShowTodosInProgress", false ) );
  mShowTodayStartingTodos->setChecked( todo.readEntry( "ShowTodayStartingTodos", false ) );
  mShowOverdueTodos->setChecked( todo.readEntry( "ShowOverdueTodos", false ) );
  mShowCompleted->setChecked( todo.readEntry( "ShowCompleted", false ) );
  mPriority->setValue( todo.readEntry( "MaxPriority", 0 ) );

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

  int days = 5;
//   switch ( mCalendarGroup->selectedId() ) {
//   case 0:
//     days = 1;
//     break;
//   case 1:
//     days = 5;
//     break;
//   case 2:
//     days = 7;
//     break;
//   case 3:
//     days = 31;
//     break;
//   case 4:
//   default:
//     days = mCustomDays->value();
//     break;
//   }
  calendar.writeEntry( "DaysToShow", days );

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
