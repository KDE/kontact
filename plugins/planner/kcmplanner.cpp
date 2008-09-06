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

void KCMPlanner::setTodo( bool state )
{
   mTodo = state;
}

// void KCMPlanner::setSd( bool state )
// {
//    mSd = state;
// }

void KCMPlanner::modified()
{
  emit changed( true );
}

void KCMPlanner::buttonClicked( bool state )
{
  mCustomDays->setEnabled( state );
}

void KCMPlanner::initGUI()
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QWidget *widget = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( widget );

  //GroupBox with General Settings
  QGroupBox *groupBox = new QGroupBox( this );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox );

  mShowRecurrence = new QCheckBox( i18n( "Show recurrences" ) );
  mShowReminder = new QCheckBox( i18n( "Show reminders" ) );
  mUnderline = new QCheckBox( i18n( "Underline Links" ) );

  boxLayout->addWidget( mShowRecurrence );
  boxLayout->addWidget( mShowReminder );
  boxLayout->addWidget( mUnderline );

  connect( mShowRecurrence, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowReminder, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mUnderline, SIGNAL(toggled(bool)), SLOT(modified()) );

  groupBox->setLayout( boxLayout );

  //GroupBox for setting number of Days
  mCalendarGroup = new QGroupBox( i18n( "Show Upcoming Events Starting" ), widget );
  QVBoxLayout *groupLayout = new QVBoxLayout( mCalendarGroup );

  mDay = new QRadioButton( i18n( "Today only" ) );

  groupLayout->addWidget( mDay );

  //Add Spinbox for choosing the days to show in summary
  QHBoxLayout *hbox = new QHBoxLayout( );
  mCalendarSpin = new QRadioButton( "Within the next" );
  hbox->addWidget( mCalendarSpin );

  mCustomDays = new QSpinBox( this );
  mCustomDays->setRange( 2, 365 );
  mCustomDays->setValue( 2 );
  mCustomDays->setSuffix( i18n( " day" ) );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  connect( mDay, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mCalendarSpin, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mCalendarSpin, SIGNAL(toggled(bool)), SLOT(buttonClicked( bool )) );
  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(modified()) );

  groupLayout->addLayout( hbox );
  mCalendarGroup->setLayout( groupLayout );

  //GroupBox for todo options
  mTodoGroup = new QGroupBox( i18n( "Show To-dos" ), widget );
  mTodoGroup->setCheckable( true );
  QVBoxLayout *todoLayout = new QVBoxLayout( mTodoGroup );

  mShowOverdueTodos = new QCheckBox( i18n( "Overdue (not completetd and beyond due-date)" ) );
  todoLayout->addWidget( mShowOverdueTodos );
  mShowTodayStartingTodos = new QCheckBox( i18n( "To-dos starting today" ) );
  todoLayout->addWidget( mShowTodayStartingTodos );
  mShowTodayEndingTodos = new QCheckBox( i18n( "To-dos ending today" ) );
  todoLayout->addWidget( mShowTodayEndingTodos );
  mShowTodosInProgress = new QCheckBox( i18n( "In-progress(started but not completetd)" ) );
  todoLayout->addWidget( mShowTodosInProgress );

  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mTodoGroup, SIGNAL(toggled(bool)), SLOT(setTodo(bool)) );
  connect( mShowTodayEndingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodosInProgress, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowTodayStartingTodos, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mShowOverdueTodos, SIGNAL(toggled(bool)), SLOT(modified()) );

  mTodoGroup->setLayout( todoLayout );

  //GroupBox for special Dates options
  mSdGroup = new QGroupBox( i18n( "Show Special Dates" ), widget );
  mSdGroup->setCheckable( true );
  QVBoxLayout *sdLayout = new QVBoxLayout( mSdGroup );

  QHBoxLayout *hbox2 = new QHBoxLayout( );
  mBirthdayCal = new QCheckBox( i18n( "Show Birthdays from Calendar" ) );
  hbox2->addWidget( mBirthdayCal );
  mBirthdayConList = new QCheckBox( i18n( "Show Birthdays from Contact List" ) );
  hbox2->addWidget( mBirthdayConList );

  sdLayout->addLayout( hbox2 );

  QHBoxLayout *hbox3 = new QHBoxLayout( );
  mAnniversariesCal = new QCheckBox( i18n( "Show Anniversaries from Calendar" ) );
  hbox3->addWidget( mAnniversariesCal );

  mAnniversariesConList = new QCheckBox( i18n( "Show Anniversaries from Contact List" ) );
  hbox3->addWidget( mAnniversariesConList );

  sdLayout->addLayout( hbox3 );

  mHolidaysCal = new QCheckBox( i18n( "Show Holidays from Calendar" ) );
  sdLayout->addWidget( mHolidaysCal );
  mSpecialOccasionsCal = new QCheckBox( i18n( "Show Special Occasions from Calendar" ) );
  sdLayout->addWidget( mSpecialOccasionsCal );

  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(modified()) );
  connect( mSdGroup, SIGNAL(toggled(bool)), SLOT(setTodo(bool)) );
  connect( mBirthdayCal, SIGNAL(toggled(bool)), SLOT(modified())  );
  connect( mBirthdayConList, SIGNAL(toggled(bool)), SLOT(modified())  );
  connect( mAnniversariesCal, SIGNAL(toggled(bool)), SLOT(modified())  );
  connect( mAnniversariesConList, SIGNAL(toggled(bool)), SLOT(modified())  );
  connect( mHolidaysCal, SIGNAL(toggled(bool)), SLOT(modified())  );
  connect( mSpecialOccasionsCal, SIGNAL(toggled(bool)), SLOT(modified())  );

  mSdGroup->setLayout( sdLayout );

  layout->addWidget( groupBox );
  layout->addWidget( mCalendarGroup );
  layout->addWidget( mTodoGroup );
  layout->addWidget( mSdGroup );

  topLayout->addWidget( widget );
  topLayout->addStretch();

}

void KCMPlanner::load()
{
  KConfig config( "plannerrc" );

  //Read general config
  KConfigGroup general = config.group( "General" );

  mShowRecurrence->setChecked( general.readEntry( "ShowRecurrence", true ) );
  mShowReminder->setChecked( general.readEntry( "ShowReminder", true ) );
  mUnderline->setChecked( general.readEntry( "underlineLink", true ) );

  //Read Calendar Config
  KConfigGroup calendar = config.group( "Calendar" );

  //Set the count of Days from config
  int days = calendar.readEntry( "DaysToShow", 1 );
  if ( days == 1 ) {
    mDay->setChecked( true );
  } else {
    mCalendarSpin->setChecked( true );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  //Read Todo Config
  KConfigGroup todo = config.group( "Todo" );

  mTodoGroup->setChecked( todo.readEntry( "Todo", true ) );
  mShowTodayEndingTodos->setChecked( todo.readEntry( "ShowTodayEndingTodos", true ) );
  mShowTodosInProgress->setChecked( todo.readEntry( "ShowTodosInProgress", true ) );
  mShowTodayStartingTodos->setChecked( todo.readEntry( "ShowTodayStartingTodos", true ) );
  mShowOverdueTodos->setChecked( todo.readEntry( "ShowOverdueTodos", true ) );

  //Read Special Dates Config
  KConfigGroup sd = config.group( "SpecialDates" );

  mSdGroup->setChecked( sd.readEntry( "SpecialDates", true ) );
  mBirthdayCal->setChecked( sd.readEntry( "BirthdayCal", true ) );
  mBirthdayConList->setChecked( sd.readEntry( "BirthdayConList", true ) );
  mAnniversariesCal->setChecked( sd.readEntry( "AnniversariesCal", true ) );
  mAnniversariesConList->setChecked( sd.readEntry( "AnniversariesConList", true ) );
  mHolidaysCal->setChecked( sd.readEntry ( "HolidaysCal", true ) );
  mSpecialOccasionsCal->setChecked( sd.readEntry( "SpecialOccasionsCal", true ) );
  emit changed( false );
}

void KCMPlanner::save()
{
  KConfig config( "plannerrc" );

  //General Setion
  KConfigGroup general = config.group( "General" );

  general.writeEntry( "ShowRecurrence", mShowRecurrence->isChecked() );
  general.writeEntry( "ShowReminder", mShowReminder->isChecked() );
  general.writeEntry( "underlineLink", mUnderline->isChecked() );

  //Calendar Section
  KConfigGroup calendar = config.group( "Calendar" );

  int days ;
  if ( mDay->isChecked() ) {
    days = 1;
  } else {
    days = mCustomDays->value();
  }
  calendar.writeEntry( "DaysToShow", days );

  //Todo Section
  KConfigGroup todo = config.group( "Todo" );

  todo.writeEntry( "Todo", mTodo );
  todo.writeEntry( "ShowTodayEndingTodos", mShowTodayEndingTodos->isChecked() );
  todo.writeEntry( "ShowTodosInProgress", mShowTodosInProgress->isChecked() );
  todo.writeEntry( "ShowTodayStartingTodos", mShowTodayStartingTodos->isChecked() );
  todo.writeEntry( "ShowOverdueTodos", mShowOverdueTodos->isChecked() );

  //SpecialDates Section
  KConfigGroup sd = config.group( "SpecialDates" );

  sd.writeEntry( "SpecialDates", mSd );
  sd.writeEntry( "BirthdayCal", mBirthdayCal->isChecked() );
  sd.writeEntry( "BirthdayConList", mBirthdayConList->isChecked() );
  sd.writeEntry( "AnniversariesCal", mAnniversariesCal->isChecked() );
  sd.writeEntry( "AnniversariesConList", mAnniversariesConList->isChecked() );
  sd.writeEntry( "HolidaysCal", mHolidaysCal->isChecked() );
  sd.writeEntry( "SpecialOccasionsCal", mSpecialOccasionsCal->isChecked() );

  config.sync();

  emit changed( false );
}

 void KCMPlanner::defaults()
{
  mShowRecurrence->setChecked( true );
  mShowReminder->setChecked( true );
  mUnderline->setChecked( true );

  mTodo = true;
  mShowTodayEndingTodos->setChecked( true );
  mShowTodosInProgress->setChecked( true );
  mShowTodayStartingTodos->setChecked( true );
  mShowOverdueTodos->setChecked( true );

  mSd = true ;
  mBirthdayCal->setChecked( true );
  mBirthdayConList->setChecked( true );
  mAnniversariesCal->setChecked( true );
  mAnniversariesConList->setChecked( true );
  mHolidaysCal->setChecked( true );
  mSpecialOccasionsCal->setChecked( true );

  emit changed( true );
}

#include "kcmplanner.moc"
