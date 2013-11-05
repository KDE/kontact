/*
  This file is part of Kontact.
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2006-2008 Oral Timocin <oral.timocin@kdemail.net>
  Copyright (C) 2009 Allen Winter <winter@kde.org>

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

#include <KAboutData>
#include <KComponentData>

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
  setupUi( this );

  customDaysChanged( 7 );

  connect( mShowRecurrence, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mShowReminder, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mUnderline, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mTodo, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mSd, SIGNAL(stateChanged(int)), SLOT(modified()) );

  connect( mDateTodayButton, SIGNAL(clicked(bool)), SLOT(modified()) );
  connect( mDateMonthButton, SIGNAL(clicked(bool)), SLOT(modified()) );
  connect( mDateRangeButton, SIGNAL(clicked(bool)), SLOT(modified()) );

  connect( mHideCompletedBox, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mHideOpenEndedBox, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mHideUnstartedBox, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mHideInProgressBox, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mHideOverdueBox, SIGNAL(stateChanged(int)), SLOT(modified()) );

  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(modified()) );
  connect( mCustomDays, SIGNAL(valueChanged(int)), SLOT(customDaysChanged(int)) );

//   connect( mBirthdayCal, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mBirthdayConList, SIGNAL(stateChanged(int)), SLOT(modified()) );
//   connect( mAnniversariesCal, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mAnniversariesConList, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mHolidaysCal, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mSpecialOccasionsCal, SIGNAL(stateChanged(int)), SLOT(modified()) );

  connect( mShowMyEventsOnly, SIGNAL(stateChanged(int)), SLOT(modified()) );
  connect( mShowMyTodosOnly, SIGNAL(stateChanged(int)), SLOT(modified()) );

  load();
}

KCMPlanner::~KCMPlanner()
{
}

void KCMPlanner::modified()
{
  emit changed( true );
}

void KCMPlanner::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMPlanner::buttonClicked( bool state )
{
  mCustomDays->setEnabled( state );
}

void KCMPlanner::load()
{
  KConfig config( "plannerrc" );

  //Read general config
  KConfigGroup group = config.group( "General" );

  mShowRecurrence->setChecked( group.readEntry( "ShowRecurrence", true ) );
  mShowReminder->setChecked( group.readEntry( "ShowReminder", true ) );
  mUnderline->setChecked( group.readEntry( "underlineLink", true ) );
  mTodo->setChecked( group.readEntry( "ShowTodo", true ) );
  mSd->setChecked( group.readEntry( "ShowSd", true ) );

  //Read Calendar Config
  group = config.group( "Calendar" );

  //Set the count of Days from config
  int days = group.readEntry( "DaysToShow", 7 );
  if ( days == 1 ) {
    mDateTodayButton->setChecked( true );
  } else if( days == 31 ) {
    mDateMonthButton->setChecked( true );
  } else {
    mDateRangeButton->setChecked( true );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  //Read Todo Config
  group = config.group( "Hide" );
  mHideInProgressBox->setChecked( group.readEntry( "InProgress", false ) );
  mHideOverdueBox->setChecked( group.readEntry( "Overdue", false ) );
  mHideCompletedBox->setChecked( group.readEntry( "Completed", true ) );
  mHideOpenEndedBox->setChecked( group.readEntry( "OpenEnded", false ) );
  mHideUnstartedBox->setChecked( group.readEntry( "NotStarted", false ) );

  //Read Special Dates Config
  group = config.group( "SpecialDates" );
//   mSdGroup->setChecked( group.readEntry( "SpecialDates", true ) );
//   mBirthdayCal->setChecked( group.readEntry( "BirthdayCal", true ) );
  mBirthdayConList->setChecked( group.readEntry( "BirthdayConList", true ) );
//   mAnniversariesCal->setChecked( group.readEntry( "AnniversariesCal", true ) );
  mAnniversariesConList->setChecked( group.readEntry( "AnniversariesConList", true ) );
  mHolidaysCal->setChecked( group.readEntry ( "HolidaysCal", true ) );
  mSpecialOccasionsCal->setChecked( group.readEntry( "SpecialOccasionsCal", true ) );

  //Read Groupware Config
  group = config.group( "Groupware" );
  mShowMyEventsOnly->setChecked( group.readEntry( "ShowMyEventsOnly", false ) );
  mShowMyTodosOnly->setChecked( group.readEntry( "ShowMyTodosOnly", false ) );

  emit changed( false );
}

void KCMPlanner::save()
{
  KConfig config( "plannerrc" );

  //General Setion
  KConfigGroup group = config.group( "General" );

  group.writeEntry( "ShowRecurrence", mShowRecurrence->isChecked() );
  group.writeEntry( "ShowReminder", mShowReminder->isChecked() );
  group.writeEntry( "underlineLink", mUnderline->isChecked() );
  group.writeEntry( "ShowTodo", mTodo->isChecked() );
  group.writeEntry( "ShowSd", mSd->isChecked() );

  //Calendar Section
  group = config.group( "Calendar" );

  int days ;
  if ( mDateTodayButton->isChecked() ) {
    days = 1;
  } else if ( mDateMonthButton->isChecked() ) {
    days = 31;
  } else {
    days = mCustomDays->value();
  }
  group.writeEntry( "DaysToShow", days );

  //Todo Section
  group = config.group( "Hide" );
  group.writeEntry( "InProgress", mHideInProgressBox->isChecked() );
  group.writeEntry( "Overdue", mHideOverdueBox->isChecked() );
  group.writeEntry( "Completed", mHideCompletedBox->isChecked() );
  group.writeEntry( "OpenEnded", mHideOpenEndedBox->isChecked() );
  group.writeEntry( "NotStarted", mHideUnstartedBox->isChecked() );

  //SpecialDates Section
  group = config.group( "SpecialDates" );

//   sd.writeEntry( "BirthdayCal", mBirthdayCal->isChecked() );
  group.writeEntry( "BirthdayConList", mBirthdayConList->isChecked() );
//   group.writeEntry( "AnniversariesCal", mAnniversariesCal->isChecked() );
  group.writeEntry( "AnniversariesConList", mAnniversariesConList->isChecked() );
  group.writeEntry( "HolidaysCal", mHolidaysCal->isChecked() );
  group.writeEntry( "SpecialOccasionsCal", mSpecialOccasionsCal->isChecked() );

  //Groupware Section
  group = config.group( "Groupware" );
  group.writeEntry( "ShowMyEventsOnly", mShowMyEventsOnly->isChecked() );
  group.writeEntry( "ShowMyTodosOnly", mShowMyTodosOnly->isChecked() );

  config.sync();

  emit changed( false );
}

void KCMPlanner::defaults()
{
  mShowRecurrence->setChecked( true );
  mShowReminder->setChecked( true );
  mUnderline->setChecked( true );
  mTodo->setChecked( true );
  mSd->setChecked( true );

  mDateRangeButton->setChecked( true );
  mCustomDays->setValue( 7 );
  mCustomDays->setEnabled( true );

  mHideInProgressBox->setChecked( false );
  mHideOverdueBox->setChecked( false );
  mHideCompletedBox->setChecked( true );
  mHideOpenEndedBox->setChecked( false );
  mHideUnstartedBox->setChecked( false );

//   mBirthdayCal->setChecked( true );
  mBirthdayConList->setChecked( true );
//   mAnniversariesCal->setChecked( true );
  mAnniversariesConList->setChecked( true );
  mHolidaysCal->setChecked( true );
  mSpecialOccasionsCal->setChecked( true );

  mShowMyEventsOnly->setChecked( false );
  mShowMyTodosOnly->setChecked( false );

  emit changed( true );
}

const KAboutData *KCMPlanner::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmplanner" ),
    0,
    ki18n( "Planner Summary Configuration Dialog" ),
    0,
    KLocalizedString(),
    KAboutData::License_GPL,
    ki18n( "Copyright © 2004 Tobias Koenig\n"
           "Copyright © 2006–2008 Oral Timocin\n"
           "Copyright © 2009–2010 Allen Winter" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
  about->addAuthor( ki18n( "Oral Timocin" ), KLocalizedString(), "o.timocin.kde@gmx.de" );

  return about;
}

