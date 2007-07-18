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

#include <Q3ButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "kcmsdsummary.h"

#include <kdemacros.h>
#include <kcomponentdata.h>

extern "C"
{
  KDE_EXPORT KCModule *create_sdsummary( QWidget *parent, const char * )
  {
    KComponentData inst( "kcmsdsummary" );
    return new KCMSDSummary( inst, parent );
  }
}

KCMSDSummary::KCMSDSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  QWidget *widget = new QWidget( parent );
  setupUi( widget );

  customDaysChanged( 7 );

  connect( mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDaysGroup,
           SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mShowFromCalGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mShowFromKABGroup,
           SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mCustomDays,
           SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays,
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
  mCustomDays->setEnabled( id == 2 );
}

void KCMSDSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18np( " day", " days", value ) );
}

void KCMSDSummary::load()
{
  KConfig config( "kcmsdsummaryrc" );

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

  config.setGroup( "Show" );

  mShowBirthdaysFromKABBox->
    setChecked( config.readEntry( "BirthdaysFromContacts", true ) );
  mShowBirthdaysFromCalBox->
    setChecked( config.readEntry( "BirthdaysFromCalendar", true ) );

  mShowAnniversariesFromKABBox->
    setChecked( config.readEntry( "AnniversariesFromContacts", true ) );
  mShowAnniversariesFromCalBox->
    setChecked( config.readEntry( "AnniversariesFromCalendar", true ) );

  mShowHolidaysFromCalBox->
    setChecked( config.readEntry( "HolidaysFromCalendar", true ) );

  mShowSpecialsFromCalBox->
    setChecked( config.readEntry( "SpecialsFromCalendar", true ) );

  emit changed( false );
}

void KCMSDSummary::save()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );

  int days;
  switch ( mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 31; break;
    case 2:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "Show" );

  config.writeEntry( "BirthdaysFromContacts",
                     mShowBirthdaysFromKABBox->isChecked() );
  config.writeEntry( "BirthdaysFromCalendar",
                     mShowBirthdaysFromCalBox->isChecked() );

  config.writeEntry( "AnniversariesFromContacts",
                     mShowAnniversariesFromKABBox->isChecked() );
  config.writeEntry( "AnniversariesFromCalendar",
                     mShowAnniversariesFromCalBox->isChecked() );

  config.writeEntry( "HolidaysFromCalendar",
                     mShowHolidaysFromCalBox->isChecked() );

  config.writeEntry( "SpecialsFromCalendar",
                     mShowSpecialsFromCalBox->isChecked() );

  config.sync();
  emit changed( false );
}

void KCMSDSummary::defaults()
{
  mDateRangeButton->setChecked( true );
  mCustomDays->setValue( 7 );
  mCustomDays->setEnabled( true );

  mShowBirthdaysFromKABBox->setChecked( true );
  mShowBirthdaysFromCalBox->setChecked( true );
  mShowAnniversariesFromKABBox->setChecked( true );
  mShowAnniversariesFromCalBox->setChecked( true );
  mShowHolidaysFromCalBox->setChecked( true );
  mShowSpecialsFromCalBox->setChecked( true );

  emit changed( true );
}

const KAboutData* KCMSDSummary::aboutData() const
{
  KAboutData *about = new KAboutData(
    I18N_NOOP( "kcmsdsummary" ), 0,
    ki18n( "Upcoming Special Dates Configuration Dialog" ),
    0, KLocalizedString(), KAboutData::License_GPL,
    ki18n( "(c) 2004-2006 Allen Winter" ) );

  about->addAuthor( ki18n("Allen Winter"), KLocalizedString(), "winter@kde.org" );
  about->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );

  return about;
}

#include "kcmsdsummary.moc"
