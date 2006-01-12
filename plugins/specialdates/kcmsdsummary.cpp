/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004-2006 Allen Winter <winter@kde.org>

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

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "kcmsdsummary.h"
#include "sdsummaryconfig_base.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_sdsummary( QWidget *parent, const char * )
  {
    KInstance *inst = new KInstance( "kcmsdsummary" );
    return new KCMSDSummary( inst, parent );
  }
}

KCMSDSummary::KCMSDSummary( KInstance *inst, QWidget *parent )
  : KCModule( inst, parent )
{
  mConfigBase = new SDSummaryConfig_Base( parent );

  customDaysChanged( 7 );

  connect( mConfigBase->mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mConfigBase->mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mConfigBase->mShowFromCalGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mConfigBase->mShowFromKABGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mConfigBase->mCustomDays,
           SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mConfigBase->mCustomDays,
           SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMSDSummary::modified()
{
  emit changed( true );
}

void KCMSDSummary::buttonClicked( int id )
{
  mConfigBase->mCustomDays->setEnabled( id == 2 );
}

void KCMSDSummary::customDaysChanged( int value )
{
  mConfigBase->mCustomDays->setSuffix( i18n( " day",  " days", value ) );
}

void KCMSDSummary::load()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );
  int days = config.readEntry( "DaysToShow", 7 );
  if ( days == 1 )
    mConfigBase->mDaysGroup->setButton( 0 );
  else if ( days == 31 )
    mConfigBase->mDaysGroup->setButton( 1 );
  else {
    mConfigBase->mDaysGroup->setButton( 2 );
    mConfigBase->mCustomDays->setValue( days );
    mConfigBase->mCustomDays->setEnabled( true );
  }

  config.setGroup( "Show" );

  mConfigBase->mShowBirthdaysFromKABBox->
    setChecked( config.readEntry( "BirthdaysFromContacts", true ) );
  mConfigBase->mShowBirthdaysFromCalBox->
    setChecked( config.readEntry( "BirthdaysFromCalendar", true ) );

  mConfigBase->mShowAnniversariesFromKABBox->
    setChecked( config.readEntry( "AnniversariesFromContacts", true ) );
  mConfigBase->mShowAnniversariesFromCalBox->
    setChecked( config.readEntry( "AnniversariesFromCalendar", true ) );

  mConfigBase->mShowHolidaysFromCalBox->
    setChecked( config.readEntry( "HolidaysFromCalendar", true ) );

  mConfigBase->mShowSpecialsFromCalBox->
    setChecked( config.readEntry( "SpecialsFromCalendar", true ) );

  emit changed( false );
}

void KCMSDSummary::save()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );

  int days;
  switch ( mConfigBase->mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 31; break;
    case 2:
    default: days = mConfigBase->mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "Show" );

  config.writeEntry( "BirthdaysFromContacts",
                     mConfigBase->mShowBirthdaysFromKABBox->isChecked() );
  config.writeEntry( "BirthdaysFromCalendar",
                     mConfigBase->mShowBirthdaysFromCalBox->isChecked() );

  config.writeEntry( "AnniversariesFromContacts",
                     mConfigBase->mShowAnniversariesFromKABBox->isChecked() );
  config.writeEntry( "AnniversariesFromCalendar",
                     mConfigBase->mShowAnniversariesFromCalBox->isChecked() );

  config.writeEntry( "HolidaysFromCalendar",
                     mConfigBase->mShowHolidaysFromCalBox->isChecked() );

  config.writeEntry( "SpecialsFromCalendar",
                     mConfigBase->mShowSpecialsFromCalBox->isChecked() );

  config.sync();
  emit changed( false );
}

void KCMSDSummary::defaults()
{
  mConfigBase->mDateRangeButton->setChecked( true );
  mConfigBase->mCustomDays->setValue( 7 );
  mConfigBase->mCustomDays->setEnabled( true );

  mConfigBase->mShowBirthdaysFromKABBox->setChecked( true );
  mConfigBase->mShowBirthdaysFromCalBox->setChecked( true );
  mConfigBase->mShowAnniversariesFromKABBox->setChecked( true );
  mConfigBase->mShowAnniversariesFromCalBox->setChecked( true );
  mConfigBase->mShowHolidaysFromCalBox->setChecked( true );
  mConfigBase->mShowSpecialsFromCalBox->setChecked( true );

  emit changed( true );
}

const KAboutData* KCMSDSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmsdsummary" ),
    I18N_NOOP( "Upcoming Special Dates Configuration Dialog" ),
    0, 0, KAboutData::License_GPL,
    I18N_NOOP( "(c) 2004-2006 Allen Winter" ) );

  about->addAuthor( "Allen Winter", 0, "winter@kde.org" );
  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmsdsummary.moc"
