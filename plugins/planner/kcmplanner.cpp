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

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdemacros.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

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
  KConfigGroup general = config.group( "General" );

  mShowRecurrence->setChecked( general.readEntry( "ShowRecurrence", true ) );
  mShowReminder->setChecked( general.readEntry( "ShowReminder", true ) );
  mUnderline->setChecked( general.readEntry( "underlineLink", true ) );
  mTodo->setChecked( general.readEntry( "ShowTodo", true ) );
  mSd->setChecked( general.readEntry( "ShowSd", true ) );

  //Read Calendar Config
  KConfigGroup calendar = config.group( "Calendar" );

  //Set the count of Days from config
  int days = calendar.readEntry( "DaysToShow", 7 );
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
  KConfigGroup hideGroup( &config, "Hide" );
  mHideInProgressBox->setChecked( hideGroup.readEntry( "InProgress", false ) );
  mHideOverdueBox->setChecked( hideGroup.readEntry( "Overdue", false ) );
  mHideCompletedBox->setChecked( hideGroup.readEntry( "Completed", true ) );
  mHideOpenEndedBox->setChecked( hideGroup.readEntry( "OpenEnded", false ) );
  mHideUnstartedBox->setChecked( hideGroup.readEntry( "NotStarted", false ) );
  //Read Special Dates Config
  KConfigGroup sd = config.group( "SpecialDates" );

//   mSdGroup->setChecked( sd.readEntry( "SpecialDates", true ) );
//   mBirthdayCal->setChecked( sd.readEntry( "BirthdayCal", true ) );
  mBirthdayConList->setChecked( sd.readEntry( "BirthdayConList", true ) );
//   mAnniversariesCal->setChecked( sd.readEntry( "AnniversariesCal", true ) );
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
  general.writeEntry( "ShowTodo", mTodo->isChecked() );
  general.writeEntry( "ShowSd", mSd->isChecked() );

  //Calendar Section
  KConfigGroup calendar = config.group( "Calendar" );

  int days ;
if ( mDateTodayButton->isChecked() ) {
    days = 1;
  } else if ( mDateMonthButton->isChecked() ) {
    days = 31;
  } else {
    days = mCustomDays->value();
  }
  calendar.writeEntry( "DaysToShow", days );

  //Todo Section
  KConfigGroup hideGroup( &config, "Hide" );
  hideGroup.writeEntry( "InProgress", mHideInProgressBox->isChecked() );
  hideGroup.writeEntry( "Overdue", mHideOverdueBox->isChecked() );
  hideGroup.writeEntry( "Completed", mHideCompletedBox->isChecked() );
  hideGroup.writeEntry( "OpenEnded", mHideOpenEndedBox->isChecked() );
  hideGroup.writeEntry( "NotStarted", mHideUnstartedBox->isChecked() );

  //SpecialDates Section
  KConfigGroup sd = config.group( "SpecialDates" );

//   sd.writeEntry( "BirthdayCal", mBirthdayCal->isChecked() );
  sd.writeEntry( "BirthdayConList", mBirthdayConList->isChecked() );
//   sd.writeEntry( "AnniversariesCal", mAnniversariesCal->isChecked() );
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

  emit changed( true );
}

const KAboutData *KCMPlanner::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmplanner" ), 0, ki18n( "Planner Summary Configuration Dialog" ),
    0, KLocalizedString(), KAboutData::License_GPL, ki18n( "(c) 2007-2008 Oral Timocin" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
  about->addAuthor( ki18n( "Oral Timocin" ), KLocalizedString(), "o.timocin.kde@gmx.de" );

  return about;
}

#include "kcmplanner.moc"
