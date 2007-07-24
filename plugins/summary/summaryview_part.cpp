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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QLayout>
#include <QTimer>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kdbusservicestarter.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <q3scrollview.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcmultidialog.h>
#include <kicon.h>
#include <kparts/componentfactory.h>
#include <kparts/event.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

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
                                  QObject *parent )
  : KParts::ReadOnlyPart( parent ),
    mCore( core ), mFrame( 0 ), mConfigAction( 0 )
{
  setComponentData( KComponentData( aboutData ) );

  loadLayout();

  initGUI( core );

  connect( KGlobalSettings::self(), SIGNAL( kdisplayPaletteChanged() ), SLOT( slotAdjustPalette() ) );
  slotAdjustPalette();

  setDate( QDate::currentDate() );
  connect( mCore, SIGNAL( dayChanged( const QDate& ) ),
           SLOT( setDate( const QDate& ) ) );

  mConfigAction  = new KAction(KIcon("configure"), i18n("&Configure Summary View..."), this);
  actionCollection()->addAction("summaryview_configure", mConfigAction );
  connect(mConfigAction, SIGNAL(triggered(bool) ), SLOT( slotConfigure() ));

  setXMLFile( "kontactsummary_part.rc" );

  QTimer::singleShot( 0, this, SLOT( slotTextChanged() ) );
}

SummaryViewPart::~SummaryViewPart()
{
  saveLayout();
}

bool SummaryViewPart::openFile()
{
  kDebug(5006) << "SummaryViewPart:openFile()" << endl;
  return true;
}

void SummaryViewPart::partActivateEvent( KParts::PartActivateEvent *event )
{
  // inform the plugins that the part has been activated so that they can
  // update the displayed information
  if ( event->activated() && ( event->part() == this ) ) {
    updateSummaries();
  }

  KParts::ReadOnlyPart::partActivateEvent( event );
}

void SummaryViewPart::updateSummaries()
{
  QMap<QString, Kontact::Summary*>::Iterator it;
  for ( it = mSummaries.begin(); it != mSummaries.end(); ++it )
    it.value()->updateSummary( false );
}

void SummaryViewPart::updateWidgets()
{
  mMainWidget->setUpdatesEnabled( false );

  delete mFrame;

  KPIMIdentities::IdentityManager idm( true, this );
  const KPIMIdentities::Identity &id = idm.defaultIdentity();

  QString currentUser = i18n( "Summary for %1", id.fullName() );
  mUsernameLabel->setText( QString::fromLatin1( "<b>%1</b>" ).arg( currentUser ) );

  mSummaries.clear();

  mFrame = new DropWidget( mMainWidget );
  connect( mFrame, SIGNAL( summaryWidgetDropped( QWidget*, QWidget*, int ) ),
           this, SLOT( summaryWidgetMoved( QWidget*, QWidget*, int ) ) );

  mMainLayout->insertWidget( 2, mFrame );

  QStringList activeSummaries;

  KConfig config( "kontact_summaryrc" );
  if ( !config.hasKey( "ActiveSummaries" ) ) {
    activeSummaries << "kontact_kmailplugin";
    activeSummaries << "kontact_specialdatesplugin";
    activeSummaries << "kontact_knotesplugin";
    activeSummaries << "kontact_korganizerplugin";
    activeSummaries << "kontact_todoplugin";
    activeSummaries << "kontact_newstickerplugin";
  } else {
    activeSummaries = config.readEntry( "ActiveSummaries" , QStringList() );
  }

  // Collect all summary widgets with a summaryHeight > 0
  QStringList loadedSummaries;

  QList<Kontact::Plugin*> plugins = mCore->pluginList();
  QList<Kontact::Plugin*>::ConstIterator end = plugins.end();
  QList<Kontact::Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = *it;
    if ( !activeSummaries.contains( plugin->identifier() )  )
      continue;

    Kontact::Summary *summary = plugin->createSummaryWidget( mFrame );
    if ( summary ) {
      if ( summary->summaryHeight() > 0 ) {
        mSummaries.insert( plugin->identifier(), summary );

        connect( summary, SIGNAL( message( const QString& ) ),
                 BroadcastStatus::instance(), SLOT( setStatusMsg( const QString& ) ) );
        connect( summary, SIGNAL( summaryWidgetDropped( QWidget*, QWidget*, int ) ),
                 this, SLOT( summaryWidgetMoved( QWidget*, QWidget*, int ) ) );

        if ( !mLeftColumnSummaries.contains( plugin->identifier() ) &&
             !mRightColumnSummaries.contains( plugin->identifier() ) ) {
          mLeftColumnSummaries.append( plugin->identifier() );
        }

        loadedSummaries.append( plugin->identifier() );
      } else {
        summary->hide();
      }
    }
  }

  // Remove all unavailable summary widgets
  {
    QStringList::Iterator strIt;
    for ( strIt = mLeftColumnSummaries.begin(); strIt != mLeftColumnSummaries.end(); ++strIt ) {
      if ( !loadedSummaries.contains( *strIt )  ) {
        strIt = mLeftColumnSummaries.erase( strIt );
        --strIt;
      }
    }
    for ( strIt = mRightColumnSummaries.begin(); strIt != mRightColumnSummaries.end(); ++strIt ) {
      if ( !loadedSummaries.contains( *strIt )  ) {
        strIt = mRightColumnSummaries.erase( strIt );
        --strIt;
      }
    }
  }

  // Add vertical line between the two rows of summary widgets.
  QFrame *vline = new QFrame( mFrame );
  vline->setFrameStyle( QFrame::VLine | QFrame::Plain );

  QHBoxLayout *layout = new QHBoxLayout( mFrame );

  mLeftColumn = new QVBoxLayout();
  layout->addItem( mLeftColumn );
  mLeftColumn->setSpacing( KDialog::spacingHint() );
  layout->addWidget( vline );
  mRightColumn = new QVBoxLayout();
  layout->addItem( mRightColumn );
  mRightColumn->setSpacing( KDialog::spacingHint() );


  QStringList::Iterator strIt;
  for ( strIt = mLeftColumnSummaries.begin(); strIt != mLeftColumnSummaries.end(); ++strIt ) {
    if ( mSummaries.contains( *strIt )  )
      mLeftColumn->addWidget( mSummaries[ *strIt ] );
  }

  for ( strIt = mRightColumnSummaries.begin(); strIt != mRightColumnSummaries.end(); ++strIt ) {
    if ( mSummaries.contains( *strIt )  )
      mRightColumn->addWidget( mSummaries[ *strIt ] );
  }

  mFrame->show();

  mMainWidget->setUpdatesEnabled( true );
  mMainWidget->update();

  mLeftColumn->addStretch();
  mRightColumn->addStretch();
}

void SummaryViewPart::summaryWidgetMoved( QWidget *target, QWidget *widget, int alignment )
{
  if ( target == widget )
    return;

  if ( target == mFrame ) {
    if ( mLeftColumn->indexOf( widget ) == -1 && mRightColumn->indexOf( widget ) == -1 )
      return;
  } else {
    if ( mLeftColumn->indexOf( target ) == -1 && mRightColumn->indexOf( target ) == -1 ||
         mLeftColumn->indexOf( widget ) == -1 && mRightColumn->indexOf( widget ) == -1 )
      return;
  }

  if ( mLeftColumn->indexOf( widget ) != -1 ) {
    mLeftColumn->remove( widget );
    mLeftColumnSummaries.removeAll( widgetName( widget ) );
  } else if ( mRightColumn->indexOf( widget ) != -1 ) {
    mRightColumn->remove( widget );
    mRightColumnSummaries.removeAll( widgetName( widget ) );
  }

  if ( target == mFrame ) {
    int pos = 0;

    if ( alignment & Qt::AlignTop )
      pos = 0;

    if ( alignment & Qt::AlignLeft ) {
      if ( alignment & Qt::AlignBottom )
        pos = mLeftColumnSummaries.count();

      mLeftColumn->insertWidget( pos, widget );
      mLeftColumnSummaries.insert( pos, widgetName( widget ) );
    } else {
      if ( alignment & Qt::AlignBottom )
        pos = mRightColumnSummaries.count();

      mRightColumn->insertWidget( pos, widget );
      mRightColumnSummaries.insert( pos, widgetName( widget ) );
    }

    return;
  }

  int targetPos = mLeftColumn->indexOf( target );
  if ( targetPos != -1 ) {
    if ( alignment == Qt::AlignBottom )
      targetPos++;

    mLeftColumn->insertWidget( targetPos, widget );
    mLeftColumnSummaries.insert( targetPos, widgetName( widget ) );
  } else {
    targetPos = mRightColumn->indexOf( target );

    if ( alignment == Qt::AlignBottom )
      targetPos++;

    mRightColumn->insertWidget( targetPos, widget );
    mRightColumnSummaries.insert( targetPos, widgetName( widget ) );
  }
}

void SummaryViewPart::slotTextChanged()
{
  emit textChanged( i18n( "What's next?" ) );
}

void SummaryViewPart::slotAdjustPalette()
{
  mMainWidget->setBackgroundRole( QPalette::Base );
}

void SummaryViewPart::setDate( const QDate& newDate )
{
  QString date( "<b>%1</b>" );
  date = date.arg( KGlobal::locale()->formatDate( newDate ) );
  mDateLabel->setText( date );
}

void SummaryViewPart::slotConfigure()
{
  KCMultiDialog dlg( mMainWidget );
  dlg.setObjectName( "ConfigDialog" );
  dlg.setModal( true );

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
    QStringList cm = it.value()->configModules();
    QStringList::ConstIterator strIt;
    for ( strIt = cm.begin(); strIt != cm.end(); ++strIt )
      if ( !(*strIt).isEmpty() && !modules.contains( *strIt ) )
        modules.append( *strIt );
  }

  return modules;
}

void SummaryViewPart::initGUI( Kontact::Core *core )
{
  QScrollArea *sa = new QScrollArea( core );

  sa->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  sa->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  sa->setWidgetResizable( true );

  mMainWidget = new QFrame;
  sa->setWidget( mMainWidget );
  mMainWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  sa->setFocusPolicy( Qt::StrongFocus );
  setWidget( sa );

  mMainLayout = new QVBoxLayout( mMainWidget );
  mMainLayout->setSpacing( KDialog::spacingHint() );
  mMainLayout->setMargin( KDialog::marginHint() );

  QHBoxLayout *hbl = new QHBoxLayout();
  mMainLayout->addItem( hbl );
  mUsernameLabel = new QLabel( mMainWidget );
  hbl->addWidget( mUsernameLabel );
  mDateLabel = new QLabel( mMainWidget );
  mDateLabel->setAlignment( Qt::AlignRight );
  hbl->addWidget( mDateLabel );

  QFrame *hline = new QFrame( mMainWidget );
  hline->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mMainLayout->insertWidget( 1, hline );

  mFrame = new DropWidget( mMainWidget );
  mMainLayout->insertWidget( 2, mFrame );

  connect( mFrame, SIGNAL( summaryWidgetDropped( QWidget*, QWidget*, int ) ),
           this, SLOT( summaryWidgetMoved( QWidget*, QWidget*, int ) ) );

  updateWidgets();
}

void SummaryViewPart::loadLayout()
{
  KConfig config( "kontact_summaryrc" );

  if ( !config.hasKey( "LeftColumnSummaries" ) ) {
    mLeftColumnSummaries << "kontact_korganizerplugin";
    mLeftColumnSummaries << "kontact_todoplugin";
    mLeftColumnSummaries << "kontact_kaddressbookplugin";
    mLeftColumnSummaries << "kontact_specialdatesplugin";
  } else {
    mLeftColumnSummaries = config.readEntry( "LeftColumnSummaries" , QStringList() );
  }

  if ( !config.hasKey( "RightColumnSummaries" ) ) {
    mRightColumnSummaries << "kontact_newstickerplugin";
  } else {
    mRightColumnSummaries = config.readEntry( "RightColumnSummaries" , QStringList() );
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
    if ( it.value() == widget )
      return it.key();
  }

  return QString();
}

#include "summaryview_part.moc"
