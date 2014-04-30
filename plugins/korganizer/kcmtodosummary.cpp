/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
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
  with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcmtodosummary.h"

#include <KAboutData>
#include <KAcceleratorManager>
#include <KComponentData>
#include <KConfigGroup>
#include <KLocalizedString>

KCModule *create_todosummary( QWidget *parent, const char * )
{
  KComponentData inst( "kcmtodosummary" );
  return new KCMTodoSummary( inst, parent );
}

KCMTodoSummary::KCMTodoSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( /*inst,*/ parent )
{
  setupUi( this );

  customDaysChanged( 7 );

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

  connect( mShowMineOnly, SIGNAL(stateChanged(int)), SLOT(modified()) );

  KAcceleratorManager::manage( this );

  load();
}

KCMTodoSummary::~KCMTodoSummary()
{
}

void KCMTodoSummary::modified()
{
  emit changed( true );
}

void KCMTodoSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMTodoSummary::load()
{
  KConfig config( QLatin1String("kcmtodosummaryrc") );
  KConfigGroup group = config.group( "Days" );

  int days = group.readEntry( "DaysToShow", 7 );
  if ( days == 1 ) {
    mDateTodayButton->setChecked( true );
  } else if ( days == 31 ) {
    mDateMonthButton->setChecked( true );
  } else {
    mDateRangeButton->setChecked( true );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  group = config.group( "Hide" );
  mHideInProgressBox->setChecked( group.readEntry( "InProgress", false ) );
  mHideOverdueBox->setChecked( group.readEntry( "Overdue", false ) );
  mHideCompletedBox->setChecked( group.readEntry( "Completed", true ) );
  mHideOpenEndedBox->setChecked( group.readEntry( "OpenEnded", false ) );
  mHideUnstartedBox->setChecked( group.readEntry( "NotStarted", false ) );

  group = config.group( "Groupware" );
  mShowMineOnly->setChecked( group.readEntry( "ShowMineOnly", false ) );

  emit changed( false );
}

void KCMTodoSummary::save()
{
  KConfig config( QLatin1String("kcmtodosummaryrc") );
  KConfigGroup group = config.group( "Days" );

  int days;
  if ( mDateTodayButton->isChecked() ) {
    days = 1;
  } else if ( mDateMonthButton->isChecked() ) {
    days = 31;
  } else {
    days = mCustomDays->value();
  }
  group.writeEntry( "DaysToShow", days );

  group = config.group( "Hide" );
  group.writeEntry( "InProgress", mHideInProgressBox->isChecked() );
  group.writeEntry( "Overdue", mHideOverdueBox->isChecked() );
  group.writeEntry( "Completed", mHideCompletedBox->isChecked() );
  group.writeEntry( "OpenEnded", mHideOpenEndedBox->isChecked() );
  group.writeEntry( "NotStarted", mHideUnstartedBox->isChecked() );

  group = config.group( "Groupware" );
  group.writeEntry( "ShowMineOnly", mShowMineOnly->isChecked() );

  config.sync();
  emit changed( false );
}

void KCMTodoSummary::defaults()
{
  mDateRangeButton->setChecked( true );
  mCustomDays->setValue( 7 );
  mCustomDays->setEnabled( true );

  mHideInProgressBox->setChecked( false );
  mHideOverdueBox->setChecked( false );
  mHideCompletedBox->setChecked( true );
  mHideOpenEndedBox->setChecked( false );
  mHideUnstartedBox->setChecked( false );

  mShowMineOnly->setChecked( false );

  emit changed( true );
}
#if 0 //QT5
const KAboutData *KCMTodoSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmtodosummary" ), 0,
    ki18n( "Pending To-dos Configuration Dialog" ),
    0, KLocalizedString(), KAboutData::License_GPL,
    ki18n( "Copyright © 2003–2004 Tobias Koenig\n"
           "Copyright © 2005–2010 Allen Winter" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ),
                    KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n( "Allen Winter" ),
                    KLocalizedString(), "winter@kde.org" );

  return about;
}
#endif
