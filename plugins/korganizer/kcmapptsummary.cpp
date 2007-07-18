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
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcomponentdata.h>

#include "kcmapptsummary.h"

#include <kdemacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_apptsummary( QWidget *parent, const char * )
  {
    KComponentData inst( "kcmapptsummary" );
    KGlobal::locale()->insertCatalog("kontact_korganizer_plugin");
    return new KCMApptSummary( inst, parent );
  }
}

KCMApptSummary::KCMApptSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  QWidget *widget = new QWidget( parent );
  setupUi( widget );

  customDaysChanged( 7 );

  connect( mDateTodayButton, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDateMonthButton, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDateRangeButton, SIGNAL( clicked( int ) ), SLOT( modified() ) );

  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

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
    mDateTodayButton->setChecked( true );
  else if ( days == 31 )
    mDateMonthButton->setChecked( true );
  else {
    mDateRangeButton->setChecked( true );
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
  if ( mDateTodayButton->isChecked() ) {
    days = 1;
  } else if ( mDateMonthButton->isChecked() ) {
    days = 31;
  } else {
    days = mCustomDays->value();
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
    I18N_NOOP( "kcmapptsummary" ), 0,
    ki18n( "Upcoming Events Configuration Dialog" ),
    0, KLocalizedString(), KAboutData::License_GPL,
    ki18n( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );
  about->addAuthor( ki18n("Allen Winter"), KLocalizedString(), "winter@kde.org" );

  return about;
}

#include "kcmapptsummary.moc"
