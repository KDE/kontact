/*
   This file is part of KDE Kontact.

   Copyright (C) 2003 Sven Lüppken <sven@kde.org>
   Copyright (C) 2003 Tobias König <tokoe@kde.org>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>

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
#include <kservice.h>
#include <ktrader.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <qscrollview.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmultidialog.h>

#include <kparts/componentfactory.h>
#include <kparts/statusbarextension.h>

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
    mCore( core ), mOptionsDialog( 0 )
{
  mStatusExt = new KParts::StatusBarExtension( this );
  setInstance( new KInstance( "kontactsummary" ) ); // ## memleak

  QScrollView *sv = new QScrollView( core );

  sv->setResizePolicy( QScrollView::AutoOneFit );
  sv->setFrameStyle( QFrame::NoFrame | QFrame::Plain );

  mFrame = new QFrame( sv->viewport(), widgetName );
  sv->addChild(mFrame);

  mFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(slotAdjustPalette()));
  slotAdjustPalette();
  sv->setFocusPolicy( QWidget::StrongFocus );
  setWidget( sv );

  mLayout = new QGridLayout( mFrame, 6, 3, KDialog::marginHint(),
                             KDialog::spacingHint() );

  getWidgets();

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "Summary" );
  connect( this, SIGNAL( textChanged( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );

  new KAction( i18n("&Configure"), "configure", 0, this,
               SLOT( slotConfigure() ), actionCollection(),
               "summaryview_configure" );

  setXMLFile( "kontactsummary_part.rc" );

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
        connect(s, SIGNAL(message(const QString&)),
                mStatusExt->statusBar(), SLOT(message(const QString&)));
        mSummaries.append( s );
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
  int currentRow = 2;
  int maxRow = 2;
  mDateLabel = new QLabel( mFrame );
  mDateLabel->setAlignment( AlignRight );
  mLayout->addMultiCellWidget( mDateLabel, 0, 0, 0, 2 );
  setDate( QDate::currentDate() );
  connect(mCore, SIGNAL( dayChanged( const QDate& ) ),
                SLOT( setDate( const QDate& ) ) );

  QFrame *hline = new QFrame( mFrame );
  hline->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mLayout->addMultiCellWidget( hline, 1, 1, 0, 2 );

  for( uint i = 0; i < mSummaries.count(); ++i ) {
    Kontact::Summary *summary = mSummaries.at( i );

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
      currentRow = 2;
      column += 2;
    } else {
      if ( i < mSummaries.count() - 1 ) {
        // Add horizontal line, when widget is not the last in the column.
        hline = new QFrame( mFrame );
        hline->setFrameStyle(/* QFrame::HLine |*/ QFrame::Plain );
        hline->setMaximumHeight( KDialog::spacingHint() );
        hline->setMinimumHeight( KDialog::spacingHint() );
        hline->hide();
        mLayout->addWidget( hline, currentRow++, column );
      }
    }
  }

  // Add vertical line between the two rows of summary widgets.
  QFrame *vline = new QFrame( mFrame );
  vline->setFrameStyle( QFrame::VLine | QFrame::Plain );
  mLayout->addMultiCellWidget( vline, 2, maxRow, 1, 1 );

  // Add line below all summaries
  hline = new QFrame( mFrame );
  hline->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mLayout->addMultiCellWidget( hline, maxRow+1, maxRow+1, 0, 2 );

  // space out remaining space to avoid ugly stretching
  mLayout->addItem(new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding ), maxRow+2, 0 );
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

void SummaryViewPart::slotAdjustPalette()
{
    mFrame->setPaletteBackgroundColor( kapp->palette().active().base() );
}

void SummaryViewPart::setDate( const QDate& newDate )
{
  QString date("<b>%1<b>");
  date = date.arg( KGlobal::locale()->formatDate( newDate ) );
  mDateLabel->setText( date );
}

void SummaryViewPart::slotConfigure()
{
  if ( !mOptionsDialog ) {
    mOptionsDialog = new KCMultiDialog( mFrame );

    QStringList modules;

    Kontact::Summary *summary;
    for( summary = mSummaries.first(); summary; summary = mSummaries.next() ) {
      QStringList cm = summary->configModules();
      QStringList::ConstIterator sit;
      for( sit = cm.begin(); sit != cm.end(); ++sit ) {
        modules.append( *sit );
      }

      connect( mOptionsDialog, SIGNAL( configCommitted() ),
               summary, SLOT( configChanged() ) );
    }

    QStringList::ConstIterator it;
    for ( it = modules.begin(); it != modules.end(); ++it ) {
      mOptionsDialog->addModule( *it );
    }
  }

  mOptionsDialog->show();
  mOptionsDialog->raise();
}

#include "summaryview_part.moc"

// vim: sw=2 sts=2 et tw=80
