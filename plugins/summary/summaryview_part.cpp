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
#include <sidebarextension.h>

#include "plugin.h"
#include "summary.h"

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
  setInstance( new KInstance( "kontactsummary" ) ); // ## memleak

  mFrame = new QFrame( core, widgetName );
  mFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(slotAdjustPalette()));
  slotAdjustPalette();
  setWidget( mFrame );

  mLayout = new QGridLayout( mFrame, 6, 3, KDialog::marginHint(),
                             KDialog::spacingHint() );

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
  int totalHeight = 0;

  // Collect all summary widgets with a summaryHeight > 0
  QPtrList<Kontact::Summary> summaries;
  QValueList<Kontact::Plugin*> plugins = mCore->pluginList();
  QValueList<Kontact::Plugin*>::ConstIterator end = plugins.end();
  QValueList<Kontact::Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = *it;
    Kontact::Summary *s = plugin->createSummaryWidget( mFrame );
    if ( s ) {
      int h = s->summaryHeight();
      kdDebug() << "Summary for " << plugin->title() << " Height: " << h
                << endl;
      if ( h ) {
        totalHeight += s->summaryHeight();
        summaries.append( s );
      } else {
        s->hide();
      }
    }
  }

  // Layout the summary widgets. Put widgets in two columns. Each widget gets as
  // many rows in the layout as Summary::summaryHeight() defines. Separator
  // lines are automatically added as appropriate.

  int column = 0;

  int currentHeight = 0;
  int currentRow = 0;
  int maxRow = 0;
  uint i;
  for( i = 0; i < summaries.count(); ++i ) {
    Kontact::Summary *summary = summaries.at( i );

    int h = summary->summaryHeight();

    // Add summary widget using as many rows of the layout as specified by
    // Kontact::Summary::summaryHeight().
    if ( h == 1 ) {
      mLayout->addWidget( summary, currentRow, column );
    } else {
      mLayout->addMultiCellWidget( summary, currentRow, currentRow + h - 1,
                                   column, column );
    }
    
    currentHeight += h;
    currentRow += h;
    
    if ( currentHeight * 2 >= totalHeight ) {
      // Start second row
      currentHeight = 0;
      maxRow = currentRow;
      currentRow = 0;
      column += 2;
    } else {
      if ( i < summaries.count() - 1 ) {
        // Add horizontal line, when widget is not the last in the column.
        QFrame *hline = new QFrame( mFrame );
        hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
        mLayout->addWidget( hline, currentRow++, column );
      }
    }
  }

  // Add vertical line between the two rows of summary widgets.
  QFrame *vline = new QFrame( mFrame );
  vline->setFrameStyle( QFrame::VLine | QFrame::Sunken );
  mLayout->addMultiCellWidget( vline, 0, maxRow, 1, 1 );
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

void SummaryViewPart::slotAdjustPalette()
{
    mFrame->setPaletteBackgroundColor( kapp->palette().active().base() );
}

#include "summaryview_part.moc"

// vim: sw=2 sts=2 et tw=80
