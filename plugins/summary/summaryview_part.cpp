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

SummaryViewPart::SummaryViewPart( Kontact::Core *core,
                                  const char *widgetName,
                                  QObject *parent, const char *name )
  : KParts::ReadOnlyPart( parent, name ), m_core( core )
{
  setInstance( new KInstance( "summaryviewpart" ) ); // ## memleak

  m_frame = new QFrame( core, widgetName );
  m_frame->setPaletteBackgroundColor( QColor( 240, 240, 240 ) );
  setWidget(m_frame);

  m_layout = new QGridLayout( m_frame, 4, 3, KDialog::marginHint(),
                              KDialog::spacingHint() );

  QFrame *frame = new QFrame( m_frame );
  frame->setFrameStyle( QFrame::VLine | QFrame::Sunken );
  m_layout->addMultiCellWidget( frame, 0, 3, 1, 1 );

  frame = new QFrame( m_frame );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  m_layout->addWidget( frame, 1, 0 );

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
  QPtrList<Kontact::Plugin> plugins = m_core->pluginList();
  Kontact::Plugin *plugin;
  for( plugin = plugins.first(); plugin; plugin = plugins.next() ) {
    QWidget *wdg = plugin->createSummaryWidget( m_frame );
    if ( plugin->identifier() == "weather" ) {
      m_layout->addWidget( wdg, 0, 0 );
    } else if ( plugin->identifier() == "mail" ) {
      m_layout->addWidget( wdg, 0, 2 );
    } else if ( plugin->identifier()  == "newsticker" ) {
      m_layout->addMultiCellWidget( wdg, 2, 3, 0, 0 );
    } else if ( plugin->identifier() == "notes" ) {
      m_layout->addMultiCellWidget( wdg, 1, 2, 2, 2 );
    } else if ( plugin->identifier() == "contacts" ) {
      m_layout->addWidget( wdg, 3, 2 );
    }
  }
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

#include "summaryview_part.moc"
