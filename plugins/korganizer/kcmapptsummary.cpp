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

#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "kcmapptsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_apptsummary( QWidget *parent, const char * )
  {
    KInstance *inst = new KInstance( "kcmapptsummary" );
    return new KCMApptSummary( inst, parent );
  }
}

KCMApptSummary::KCMApptSummary( KInstance *inst, QWidget *parent )
  : KCModule( inst, parent )
{
  QWidget *widget = new QWidget( parent );
  setupUi( widget );

  customDaysChanged( 7 );

  connect( mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mCustomDays,
           SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays,
           SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

  KAcceleratorManager::manage( this );

  load();
}
KCMApptSummary::~KCMApptSummary()
{
}


void KCMApptSummary::modified()
{
  emit changed( true );
}

void KCMApptSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 2 );
}

void KCMApptSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMApptSummary::load()
{
  KConfig config( "kcmapptsummaryrc" );

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

  emit changed( false );
}

void KCMApptSummary::save()
{
  KConfig config( "kcmapptsummaryrc" );

  config.setGroup( "Days" );
  int days;
  switch ( mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 31; break;
    case 2:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.sync();

  emit changed( false );
}

void KCMApptSummary::defaults()
{
  mDateRangeButton->setChecked( true );
  mCustomDays->setValue( 7 );
  mCustomDays->setEnabled( true );

  emit changed( true );
}

const KAboutData* KCMApptSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmapptsummary" ),
    I18N_NOOP( "Upcoming Events Configuration Dialog" ),
    0, 0, KAboutData::License_GPL,
    I18N_NOOP( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  about->addAuthor( "Allen Winter", 0, "winter@kde.org" );

  return about;
}

#include "kcmapptsummary.moc"
