/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2005-2006 Allen Winter <winter@kde.org>

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

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "kcmtodosummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_todosummary( QWidget *parent, const char * )
  {
    KInstance *inst = new KInstance( "kcmtodosummary" );
    return new KCMTodoSummary( inst, parent );
  }
}

KCMTodoSummary::KCMTodoSummary( KInstance *inst,  QWidget *parent )
  : KCModule( inst, parent )
{
  QWidget*widget = new QWidget( parent );
  setupUi( widget );

  customDaysChanged( 7 );

  connect( mDaysGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDaysGroup, SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mHideGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

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

void KCMTodoSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 2 );
}

void KCMTodoSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMTodoSummary::load()
{
  KConfig config( "kcmtodosummaryrc" );

  config.setGroup( "Days" );
  int days = config.readEntry( "DaysToShow", 7 );
  if ( days == 1 )
    mDaysGroup->setButton( 0 );
  else if ( days == 31 )
    mDaysGroup->setButton( 1 );
  else {
    mDaysGroup->setButton( 2 );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  config.setGroup( "Hide" );
  mHideInProgressBox->setChecked( config.readEntry( "InProgress", false ) );
  mHideOverdueBox->setChecked( config.readEntry( "Overdue", false ) );
  mHideCompletedBox->setChecked( config.readEntry( "Completed", true ) );
  mHideOpenEndedBox->setChecked( config.readEntry( "OpenEnded", false ) );
  mHideUnstartedBox->setChecked( config.readEntry( "NotStarted", false ) );

  emit changed( false );
}

void KCMTodoSummary::save()
{
  KConfig config( "kcmtodosummaryrc" );

  config.setGroup( "Days" );
  int days;
  switch ( mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 31; break;
    case 2:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "Hide" );
  config.writeEntry( "InProgress", mHideInProgressBox->isChecked() );
  config.writeEntry( "Overdue", mHideOverdueBox->isChecked() );
  config.writeEntry( "Completed", mHideCompletedBox->isChecked() );
  config.writeEntry( "OpenEnded", mHideOpenEndedBox->isChecked() );
  config.writeEntry( "NotStarted", mHideUnstartedBox->isChecked() );

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

  emit changed( true );
}

const KAboutData* KCMTodoSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmtodosummary" ),
    I18N_NOOP( "Pending To-dos Configuration Dialog" ),
    0, 0, KAboutData::License_GPL,
    I18N_NOOP( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  about->addAuthor( "Allen Winter", 0, "winter@kde.org" );

  return about;
}

#include "kcmtodosummary.moc"
