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

#include "kcmkorgsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_korgsummary( QWidget *parent, const char * )
  {
    return new KCMKOrgSummary( parent, "kcmkorgsummary" );
  }
}

KCMKOrgSummary::KCMKOrgSummary( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  customDaysChanged( 1 );

  connect( mCalendarGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mCalendarGroup, SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  connect( mTodoGroup, SIGNAL( clicked( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mCustomDays, SIGNAL( valueChanged( int ) ), SLOT( customDaysChanged( int ) ) );

  KAcceleratorManager::manage( this );

  load();
  
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkorgsummary" ),
                                      I18N_NOOP( "Schedule Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKOrgSummary::modified()
{
  emit changed( true );
}

void KCMKOrgSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 4 );
}

void KCMKOrgSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18n( " day",  " days", value ) );
}

void KCMKOrgSummary::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mCalendarGroup = new QButtonGroup( 0, Vertical, i18n( "Calendar" ), this );
  QVBoxLayout *boxLayout = new QVBoxLayout( mCalendarGroup->layout(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "How many days should the calendar display at once?" ), mCalendarGroup );
  boxLayout->addWidget( label );

  QRadioButton *button = new QRadioButton( i18n( "One day" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "Five days" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One week" ), mCalendarGroup );
  boxLayout->addWidget( button );

  button = new QRadioButton( i18n( "One month" ), mCalendarGroup );
  boxLayout->addWidget( button );

  QHBoxLayout *hbox = new QHBoxLayout( boxLayout, KDialog::spacingHint() );

  button = new QRadioButton( "", mCalendarGroup );
  hbox->addWidget( button );

  mCustomDays = new QSpinBox( 1, 365, 1, mCalendarGroup );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  hbox->addStretch( 1 );

  layout->addWidget( mCalendarGroup );

  mTodoGroup = new QButtonGroup( 2, Horizontal, i18n( "To-dos" ), this );
  new QRadioButton( i18n( "Show all tasks" ), mTodoGroup );
  new QRadioButton( i18n( "Show today's tasks" ), mTodoGroup );

  layout->addWidget( mTodoGroup );
}

void KCMKOrgSummary::load()
{
  KConfig config( "kcmkorgsummaryrc" );

  config.setGroup( "Calendar" );
  int days = config.readNumEntry( "DaysToShow", 1 );
  if ( days == 1 )
    mCalendarGroup->setButton( 0 );
  else if ( days == 5 )
    mCalendarGroup->setButton( 1 );
  else if ( days == 7 )
    mCalendarGroup->setButton( 2 );
  else if ( days == 31 )
    mCalendarGroup->setButton( 3 );
  else {
    mCalendarGroup->setButton( 4 );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  config.setGroup( "Todo" );
  bool allTodos = config.readBoolEntry( "ShowAllTodos", false );

  if ( allTodos )
    mTodoGroup->setButton( 0 );
  else
    mTodoGroup->setButton( 1 );

  emit changed( false );
}

void KCMKOrgSummary::save()
{
  KConfig config( "kcmkorgsummaryrc" );

  config.setGroup( "Calendar" );

  int days;
  switch ( mCalendarGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 5; break;
    case 2: days = 7; break;
    case 3: days = 31; break;
    case 4:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "Todo" );
  config.writeEntry( "ShowAllTodos", mTodoGroup->selectedId() == 0 );

  config.sync();

  emit changed( false );
}

void KCMKOrgSummary::defaults()
{
  mCalendarGroup->setButton( 0 );
  mTodoGroup->setButton( 1 );

  emit changed( true );
}

#include "kcmkorgsummary.moc"
