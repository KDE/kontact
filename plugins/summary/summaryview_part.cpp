/* This file is part of the KDE project
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

#include "summaryview_part.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qtimer.h>

#include <sidebarextension.h>
#include "plugin.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <dcopclient.h>
#include <kdcopservicestarter.h>
#include <ktrader.h>
#include <kservice.h>
#include <kparts/componentfactory.h>
#include <infoextension.h>

namespace Kontact
{
     class MainWindow;
};

SummaryViewPart::SummaryViewPart( const QPtrList<Kontact::Plugin>& plugins,
                                  QWidget* parentWidget, const char* widgetName,
                                  QObject *parent, const char *name )
  : KParts::ReadOnlyPart(parent, name )
{
	setInstance( new KInstance("summaryviewpart") ); // ## memleak

	m_frame = new QFrame( parentWidget, widgetName );
	m_frame->setPaletteBackgroundColor( QColor( 240, 240, 240 ) );
	setWidget(m_frame);

	m_layout = new QGridLayout( m_frame, 3, 3, KDialog::marginHint(),
                              KDialog::spacingHint() );
	//m_layout->setSpacing( 50 ); We should look later which spacing is appropriate here

  QFrame *frame = new QFrame( m_frame );
  frame->setFrameStyle( QFrame::VLine | QFrame::Sunken );
  m_layout->addMultiCellWidget( frame, 0, 2, 1, 1 );

  frame = new QFrame( m_frame );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  m_layout->addWidget( frame, 1, 0 );

	setXMLFile("summaryparttui.rc");
  //new KAction( "new contact (test)", 0, this, SLOT( newContact() ), actionCollection(), "test_deleteevent" );
	//new KParts::SideBarExtension( label, this, "sbe");
	m_plugins = plugins;
	kapp->dcopClient()->setNotifications( true );

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
	kdDebug() << "Adding the widgets..." << endl;
	Kontact::Plugin *plugin;
	for( plugin = m_plugins.first(); plugin; plugin = m_plugins.next() ) {
		QWidget *wdg = plugin->createSummaryWidget( m_frame );
    if ( QString( "weather" ).compare( plugin->name() ) == 0 ) {
      m_layout->addWidget( wdg, 0, 0 );
    } else if ( QString( "kmail" ).compare( plugin->name() ) == 0 ) {
      m_layout->addWidget( wdg, 0, 2 );
    } else if ( QString( "kaddressbook" ).compare( plugin->name() ) == 0 ) {
      m_layout->addWidget( wdg, 2, 0 );
    } else if ( QString( "knotes" ).compare( plugin->name() ) == 0 ) {
      m_layout->addWidget( wdg, 2, 2 );
    }
  }
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

#include "summaryview_part.moc"
