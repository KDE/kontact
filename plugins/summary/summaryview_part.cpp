/*
   This file is part of KDE Kontact.

   Copyright (C) 2003 Sven Lüppken <sven@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kservice.h>
#include <ktrader.h>

#include <infoextension.h>
#include "plugin.h"
#include <sidebarextension.h>

#include "summaryview_part.h"

namespace Kontact
{
  class MainWindow;
}

SummaryViewPart::SummaryViewPart( Kontact::Core *core, const char *widgetName,
                                  QObject *parent, const char *name )
  : KParts::ReadOnlyPart( parent, name ),
    mCore( core )
{
  setInstance( new KInstance( "summaryviewpart" ) ); // ## memleak

  mFrame = new QFrame( core, widgetName );
  mFrame->setPaletteBackgroundColor( QColor( 240, 240, 240 ) );
  setWidget( mFrame );

  mLayout = new QGridLayout( mFrame, 4, 3, KDialog::marginHint(),
                             KDialog::spacingHint() );

  QFrame *frame = new QFrame( mFrame );
  frame->setFrameStyle( QFrame::VLine | QFrame::Sunken );
  mLayout->addMultiCellWidget( frame, 0, 3, 1, 1 );

  frame = new QFrame( mFrame );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  mLayout->addWidget( frame, 1, 0 );

  getWidgets();

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "Summary" );
  connect( this, SIGNAL( textChanged( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );

  QTimer::singleShot( 0, this, SLOT( slotTextChanged() ) );
}

SummaryViewPart::~SummaryViewPart()
{
  kapp->dcopClient()->setNotifications( false );
}

bool SummaryViewPart::openFile()
{
  kdDebug(5006) << "SummaryViewPart:openFile()" << endl;
  return true;
}

void SummaryViewPart::getWidgets()
{
  QPtrList<Kontact::Plugin> plugins = mCore->pluginList();
  Kontact::Plugin *plugin;
  for ( plugin = plugins.first(); plugin; plugin = plugins.next() ) {
    QWidget *wdg = plugin->createSummaryWidget( mFrame );
    kdDebug(5006) <<  "identifier=" << plugin->identifier() << endl;
    if ( plugin->identifier() == "weather" ) {
      mLayout->addWidget( wdg, 0, 0 );
    } else if ( plugin->identifier() == "mails" ) {
      mLayout->addWidget( wdg, 0, 2 );
    } else if ( plugin->identifier()  == "newsticker" ) {
      mLayout->addMultiCellWidget( wdg, 2, 3, 0, 0 );
    } else if ( plugin->identifier() == "notes" ) {
      mLayout->addMultiCellWidget( wdg, 1, 2, 2, 2 );
    } else if ( plugin->identifier() == "contacts" ) {
      mLayout->addWidget( wdg, 3, 2 );
    }
  }
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

#include "summaryview_part.moc"
