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
  : KPIM::Part( parent, name ),
    mCore( core ), mFrame( 0 ), mConfigAction( 0 )
{
  setInstance( new KInstance( aboutData ) );

  loadLayout();

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
  saveLayout();
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
    QMap<QString, Kontact::Summary*>::Iterator it;
    for ( it = mSummaries.begin(); it != mSummaries.end(); ++it )
      it.data()->updateSummary( false );
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

  QStringList activeSummaries;

  KConfig config( "kontact_summaryrc" );
  if ( !config.hasKey( "ActiveSummaries" ) ) {
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

    Kontact::Summary *summary = plugin->createSummaryWidget( mFrame );
    if ( summary ) {
      if ( summary->summaryHeight() > 0 ) {
        mSummaries.insert( plugin->identifier(), summary );

        connect( summary, SIGNAL( message( const QString& ) ),
                 BroadcastStatus::instance(), SLOT( setStatusMsg( const QString& ) ) );
        connect( summary, SIGNAL( summaryWidgetDropped( QWidget*, QWidget* ) ),
                 this, SLOT( summaryWidgetMoved( QWidget*, QWidget* ) ) );

        if ( !mLeftColumnSummaries.contains( plugin->identifier() ) &&
             !mRightColumnSummaries.contains( plugin->identifier() ) ) {
          mLeftColumnSummaries.append( plugin->identifier() );
        }
      } else {
        summary->hide();
      }
    }
  }

  // Add vertical line between the two rows of summary widgets.
  QFrame *vline = new QFrame( mFrame );
  vline->setFrameStyle( QFrame::VLine | QFrame::Plain );

  QHBoxLayout *layout = new QHBoxLayout( mFrame );

  mLeftColumn = new QVBoxLayout( layout, KDialog::spacingHint() );
  layout->addWidget( vline );
  mRightColumn = new QVBoxLayout( layout, KDialog::spacingHint()  );

  QStringList::Iterator strIt;
  for ( strIt = mLeftColumnSummaries.begin(); strIt != mLeftColumnSummaries.end(); ++strIt ) {
    if ( mSummaries.find( *strIt ) != mSummaries.end() )
      mLeftColumn->addWidget( mSummaries[ *strIt ] );
  }

  for ( strIt = mRightColumnSummaries.begin(); strIt != mRightColumnSummaries.end(); ++strIt ) {
    if ( mSummaries.find( *strIt ) != mSummaries.end() )
      mRightColumn->addWidget( mSummaries[ *strIt ] );
  }

  mFrame->show();

  mMainWidget->setUpdatesEnabled( true );
  mMainWidget->update();
}

void SummaryViewPart::summaryWidgetMoved( QWidget *target, QWidget *widget )
{
  if ( target == widget )
    return;

  if ( mLeftColumn->findWidget( target ) == -1 && mRightColumn->findWidget( target ) == -1 ||
       mLeftColumn->findWidget( widget ) == -1 && mRightColumn->findWidget( widget ) == -1 ) {
    return;
  }

  if ( mLeftColumn->findWidget( widget ) != -1 ) {
    mLeftColumn->remove( widget );
    mLeftColumnSummaries.remove( widgetName( widget ) );
  } else if ( mRightColumn->findWidget( widget ) != -1 ) {
    mRightColumn->remove( widget );
    mRightColumnSummaries.remove( widgetName( widget ) );
  }

  int targetPos = mLeftColumn->findWidget( target );
  if ( targetPos != -1 ) {
    mLeftColumn->insertWidget( targetPos, widget );
    mLeftColumnSummaries.insert( mLeftColumnSummaries.at( targetPos ), widgetName( widget ) );
  } else {
    targetPos = mRightColumn->findWidget( target );
    mRightColumn->insertWidget( targetPos, widget );
    mRightColumnSummaries.insert( mRightColumnSummaries.at( targetPos ), widgetName( widget ) );
  }
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

  QStringList::ConstIterator strIt;
  for ( strIt = modules.begin(); strIt != modules.end(); ++strIt )
    dlg.addModule( *strIt );

  dlg.exec();
}

QStringList SummaryViewPart::configModules() const
{
  QStringList modules;

  QMap<QString, Kontact::Summary*>::ConstIterator it;
  for ( it = mSummaries.begin(); it != mSummaries.end(); ++it ) {
    QStringList cm = it.data()->configModules();
    QStringList::ConstIterator strIt;
    for ( strIt = cm.begin(); strIt != cm.end(); ++strIt )
      if ( !(*strIt).isEmpty() && !modules.contains( *strIt ) )
        modules.append( *strIt );
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

void SummaryViewPart::loadLayout()
{
  KConfig config( "kontact_summaryrc" );

  if ( !config.hasKey( "LeftColumnSummaries" ) ) {
    mLeftColumnSummaries << "kontact_korganizerplugin";
    mLeftColumnSummaries << "kontact_todoplugin";
    mLeftColumnSummaries << "kontact_kaddressbookplugin";
  } else {
    mLeftColumnSummaries = config.readListEntry( "LeftColumnSummaries" );
  }

  if ( !config.hasKey( "RightColumnSummaries" ) ) {
    mRightColumnSummaries << "kontact_newstickerplugin";
  } else {
    mRightColumnSummaries = config.readListEntry( "RightColumnSummaries" );
  }
}

void SummaryViewPart::saveLayout()
{
  KConfig config( "kontact_summaryrc" );

  config.writeEntry( "LeftColumnSummaries", mLeftColumnSummaries );
  config.writeEntry( "RightColumnSummaries", mRightColumnSummaries );

  config.sync();
}

QString SummaryViewPart::widgetName( QWidget *widget ) const
{
  QMap<QString, Kontact::Summary*>::ConstIterator it;
  for ( it = mSummaries.begin(); it != mSummaries.end(); ++it ) {
    if ( it.data() == widget )
      return it.key();
  }

  return QString::null;
}

#include "summaryview_part.moc"
