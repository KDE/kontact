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
#include <kconfig.h>
#include <kdcopservicestarter.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <ktrader.h>
#include <kstandarddirs.h>
#include <qscrollview.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmultidialog.h>

#include <kparts/componentfactory.h>
#include <kparts/event.h>

#include <infoextension.h>
#include <sidebarextension.h>

#include "plugin.h"
#include "summary.h"

#include "summaryview_part.h"

#include "broadcaststatus.h"
using KPIM::BroadcastStatus;

namespace Kontact
{
  class MainWindow;
}

SummaryViewPart::SummaryViewPart( Kontact::Core *core, const char*,
                                  const KAboutData *aboutData,
                                  QObject *parent, const char *name )
  : KParts::ReadOnlyPart( parent, name ),
    mCore( core ), mFrame( 0 ), mConfigAction( 0 )
{
  setInstance( new KInstance( aboutData ) );

  initGUI( core );

  connect( kapp, SIGNAL( kdisplayPaletteChanged() ), SLOT( slotAdjustPalette() ) );
  slotAdjustPalette();

  setDate( QDate::currentDate() );
  connect( mCore, SIGNAL( dayChanged( const QDate& ) ),
           SLOT( setDate( const QDate& ) ) );

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "Summary" );
  connect( this, SIGNAL( textChanged( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );

  mConfigAction = new KAction( i18n( "&Configure Summary View..." ),
                               "configure", 0, this,
                               SLOT( slotConfigure() ), actionCollection(),
                               "summaryview_configure" );

  setXMLFile( "kontactsummary_part.rc" );

  QTimer::singleShot( 0, this, SLOT( slotTextChanged() ) );
}

SummaryViewPart::~SummaryViewPart()
{
}

bool SummaryViewPart::openFile()
{
  kdDebug(5006) << "SummaryViewPart:openFile()" << endl;
  return true;
}

void SummaryViewPart::partActivateEvent( KParts::PartActivateEvent *event )
{
  // inform the plugins that the part has been activated so that they can
  // update the displayed information
  if ( event->activated() && ( event->part() == this ) ) {
    QPtrListIterator<Kontact::Summary> it( mSummaries );
    for ( ; it.current(); ++it ) {
      it.current()->updateSummary( false );
    }
  }

  KParts::ReadOnlyPart::partActivateEvent( event );
}

void SummaryViewPart::updateWidgets()
{
  mMainWidget->setUpdatesEnabled( false );

  delete mFrame;

  mSummaries.clear();

  mFrame = new QFrame( mMainWidget );
  mMainLayout->insertWidget( 2, mFrame );

  int totalHeight = 0;

  QStringList activeSummaries;

  KConfig config( "kontact_summaryrc" );
  if ( !config.hasKey( "ActiveSummaries" ) ) {
    activeSummaries << "kontact_kmailplugin";
    activeSummaries << "kontact_kaddressbookplugin";
    activeSummaries << "kontact_korganizerplugin";
    activeSummaries << "kontact_todoplugin";
    activeSummaries << "kontact_newstickerplugin";
  } else {
    activeSummaries = config.readListEntry( "ActiveSummaries" );
  }

  // Collect all summary widgets with a summaryHeight > 0
  QValueList<Kontact::Plugin*> plugins = mCore->pluginList();
  QValueList<Kontact::Plugin*>::ConstIterator end = plugins.end();
  QValueList<Kontact::Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = *it;
    if ( activeSummaries.find( plugin->identifier() ) == activeSummaries.end() )
      continue;

    Kontact::Summary *s = plugin->createSummaryWidget( mFrame );
    if ( s ) {
      int h = s->summaryHeight();
      kdDebug(5602) << "Summary for " << plugin->title() << " Height: " << h
                << endl;
      if ( h ) {
        totalHeight += s->summaryHeight();
        connect( s, SIGNAL( message( const QString& ) ),
                 BroadcastStatus::instance(), SLOT( setStatusMsg( const QString& ) ) );
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
  int currentRow = 0;
  int maxRow = 0;

  QGridLayout *layout = new QGridLayout( mFrame, 6, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  for( uint i = 0; i < mSummaries.count(); ++i ) {
    Kontact::Summary *summary = mSummaries.at( i );

    int h = summary->summaryHeight();

    // Add summary widget using as many rows of the layout as specified by
    // Kontact::Summary::summaryHeight().
    if ( h == 1 ) {
      layout->addWidget( summary, currentRow, column );
    } else {
      layout->addMultiCellWidget( summary, currentRow, currentRow + h - 1,
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
    }
  }

  // Add vertical line between the two rows of summary widgets.
  QFrame *vline = new QFrame( mFrame );
  vline->setFrameStyle( QFrame::VLine | QFrame::Plain );
  layout->addMultiCellWidget( vline, 0, maxRow, 1, 1 );

  // space out remaining space to avoid ugly stretching
  layout->addItem( new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding,
                   QSizePolicy::MinimumExpanding ), maxRow, 0 );

  mFrame->show();

  mMainWidget->setUpdatesEnabled( true );
  mMainWidget->update();
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

void SummaryViewPart::slotAdjustPalette()
{
  mMainWidget->setPaletteBackgroundColor( kapp->palette().active().base() );
}

void SummaryViewPart::setDate( const QDate& newDate )
{
  QString date( "<b>%1<b>" );
  date = date.arg( KGlobal::locale()->formatDate( newDate ) );
  mDateLabel->setText( date );
}

void SummaryViewPart::slotConfigure()
{
  KCMultiDialog dlg( mMainWidget, "ConfigDialog", true );

  QStringList modules = configModules();
  modules.prepend( "kcmkontactsummary.desktop" );
  connect( &dlg, SIGNAL( configCommitted() ),
           this, SLOT( updateWidgets() ) );

  Kontact::Summary *summary;
  for ( summary = mSummaries.first(); summary; summary = mSummaries.next() )
    connect( &dlg, SIGNAL( configCommitted() ),
             summary, SLOT( configChanged() ) );

  QStringList::ConstIterator it;
  for ( it = modules.begin(); it != modules.end(); ++it )
    dlg.addModule( *it );

  dlg.exec();
}

QStringList SummaryViewPart::configModules() const
{
  QStringList modules;

  QPtrListIterator<Kontact::Summary> it( mSummaries );
  while ( it.current() ) {
    QStringList cm = it.current()->configModules();
    QStringList::ConstIterator sit;
    for ( sit = cm.begin(); sit != cm.end(); ++sit )
      if ( !modules.contains( *sit ) )
        modules.append( *sit );

    ++it;
  }

  return modules;
}

void SummaryViewPart::initGUI( Kontact::Core *core )
{
  QScrollView *sv = new QScrollView( core );

  sv->setResizePolicy( QScrollView::AutoOneFit );
  sv->setFrameStyle( QFrame::NoFrame | QFrame::Plain );

  mMainWidget = new QFrame( sv->viewport() );
  sv->addChild( mMainWidget );
  mMainWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  sv->setFocusPolicy( QWidget::StrongFocus );
  setWidget( sv );

  mMainLayout = new QVBoxLayout( mMainWidget,KDialog::marginHint(),
                                 KDialog::spacingHint() );

  mDateLabel = new QLabel( mMainWidget );
  mDateLabel->setAlignment( AlignRight );
  mMainLayout->insertWidget( 0, mDateLabel );

  QFrame *hline = new QFrame( mMainWidget );
  hline->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mMainLayout->insertWidget( 1, hline );

  mFrame = new QFrame( mMainWidget );
  mMainLayout->insertWidget( 2, mFrame );

  updateWidgets();
}

#include "summaryview_part.moc"
