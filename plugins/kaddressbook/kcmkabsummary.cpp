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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kaccelmanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "kcmkabsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kabsummary( QWidget *parent, const char * )
  {
    return new KCMKABSummary( parent, "kcmkabsummary" );
  }
}

KCMKABSummary::KCMKABSummary( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  customDaysChanged( 1 );

  connect( mDaysGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mDaysGroup, SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mShowGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

  KAcceleratorManager::manage( this );

  load();
  
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabsummary" ),
                                      I18N_NOOP( "Address Book Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKABSummary::modified()
{
  emit changed( true );
}

void KCMKABSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 4 );
}

void KCMKABSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18n( " day",  " days", value ) );
}

void KCMKABSummary::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mDaysGroup = new QButtonGroup( 0, Vertical, i18n( "Address Book Summary" ), this );
  QVBoxLayout *boxLayout = new QVBoxLayout( mDaysGroup->layout(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "How many days should the address book summary display at once?" ), mDaysGroup );
  boxLayout->addWidget( label );

  QRadioButton *button = new QRadioButton( i18n( "One day" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "Five days" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One week" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One month" ), mDaysGroup );
  boxLayout->addWidget( button );

  QHBoxLayout *hbox = new QHBoxLayout( boxLayout, KDialog::spacingHint() );

  button = new QRadioButton( "", mDaysGroup );
  hbox->addWidget( button );

  mCustomDays = new QSpinBox( 1, 365, 1, mDaysGroup );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  hbox->addStretch( 1 );

  layout->addWidget( mDaysGroup );

  mShowGroup = new QButtonGroup( 2, Horizontal, i18n( "Event Types" ), this );
  mShowBirthdays = new QCheckBox( i18n( "Show all birthdays" ), mShowGroup );
  mShowAnniversaries = new QCheckBox( i18n( "Show all anniversaries" ), mShowGroup );

  layout->addWidget( mShowGroup );
}

void KCMKABSummary::load()
{
  KConfig config( "kcmkabsummaryrc" );

  config.setGroup( "Days" );
  int days = config.readNumEntry( "DaysToShow", 7 );
  if ( days == 1 )
    mDaysGroup->setButton( 0 );
  else if ( days == 5 )
    mDaysGroup->setButton( 1 );
  else if ( days == 7 )
    mDaysGroup->setButton( 2 );
  else if ( days == 31 )
    mDaysGroup->setButton( 3 );
  else {
    mDaysGroup->setButton( 4 );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  config.setGroup( "EventTypes" );
  mShowBirthdays->setChecked( config.readBoolEntry( "ShowBirthdays", true ) );
  mShowAnniversaries->setChecked( config.readBoolEntry( "ShowAnniversaries", true ) );

  emit changed( false );
}

void KCMKABSummary::save()
{
  KConfig config( "kcmkabsummaryrc" );

  config.setGroup( "Days" );

  int days;
  switch ( mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 5; break;
    case 2: days = 7; break;
    case 3: days = 31; break;
    case 4:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "EventTypes" );
  config.writeEntry( "ShowBirthdays", mShowBirthdays->isChecked() );
  config.writeEntry( "ShowAnniversaries", mShowAnniversaries->isChecked() );

  config.sync();

  emit changed( false );
}

void KCMKABSummary::defaults()
{
  mDaysGroup->setButton( 7 );
  mShowBirthdays->setChecked( true );
  mShowAnniversaries->setChecked( true );

  emit changed( true );
}

#include "kcmkabsummary.moc"
