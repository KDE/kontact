/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include <qcombobox.h>
#include <qhbox.h>
#include <qimage.h>
#include <qobjectlist.h>
#include <qsplitter.h>
#include <qwhatsthis.h>
#include <qtimer.h>
#include <qprogressbar.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kguiitem.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klibloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kplugininfo.h>
#include <kpopupmenu.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <ktip.h>
#include <ktrader.h>
#include <ksettings/componentsdialog.h>
#include <kstringhandler.h>

#include <infoextension.h>

#include "aboutdialog.h"
#include "iconsidepane.h"
#include "mainwindow.h"
#include "plugin.h"
#include "prefs.h"
#include "sidepane.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "broadcaststatus.h"
#include "splash.h"

using namespace Kontact;

MainWindow::MainWindow(Kontact::Splash *splash)
  : Kontact::Core(), mTopWidget( 0 ), mHeaderText( 0 ), mHeaderPixmap( 0 ), mSplitter( 0 ),
    mSplash( splash ), mCurrentPlugin( 0 ), mLastInfoExtension( 0 ), mAboutDialog( 0 ),
    mReallyClose( false )
{
  initGUI();
  QTimer::singleShot( 0, this, SLOT(initObject()) );
}

void MainWindow::initGUI()
{
  initWidgets();
  setupActions();
  setHelpMenuEnabled( false );
  KHelpMenu *helpMenu = new KHelpMenu( this, 0, true, actionCollection() );
  connect( helpMenu, SIGNAL( showAboutApplication() ),
           SLOT( showAboutDialog() ) );

  KStdAction::keyBindings( this, SLOT( configureShortcuts() ), actionCollection() );
  KStdAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
  setXMLFile( "kontactui.rc" );

  setStandardToolBarMenuEnabled( true );

  createGUI( 0 );

  resize( 700, 520 ); // initial size to prevent a scrollbar in sidepane
  setAutoSaveSettings();

}


void MainWindow::initObject()
{
  KTrader::OfferList offers = KTrader::self()->query(
      QString::fromLatin1( "Kontact/Plugin" ),
      QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  mPluginInfos = KPluginInfo::fromServices( offers, Prefs::self()->config(), "Plugins" );

  QProgressBar *bar = mSplash->progressBar();
  int count = 3;
  count += mPluginInfos.count();
  bar->setTotalSteps( count );

  KPluginInfo::List::Iterator it;
  for ( it = mPluginInfos.begin(); it != mPluginInfos.end(); ++it ) {
    ( *it )->load();
    bar->setProgress( bar->progress() + 1 );
  }


  // prepare the part manager
  mPartManager = new KParts::PartManager( this );
  connect( mPartManager, SIGNAL( activePartChanged( KParts::Part* ) ),
           this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

  loadPlugins();

  bar->setProgress( bar->progress() + 1 );

  if ( mSidePane )
    mSidePane->updatePlugins();

  // flush paint events
  kapp->processEvents();

  bar->setProgress( bar->progress() + 1 );

  KSettings::Dispatcher::self()->registerInstance( instance(), this,
                                                   SLOT( updateConfig() ) );

  loadSettings();

  bar->setProgress( bar->progress() + 1 );

  statusBar()->show();

  // we are done - remove splash
  delete mSplash;
  mSplash=0;

  showTip( false );

  // done initializing
  slotShowStatusMsg( QString::null );

  connect( KPIM::BroadcastStatus::instance(), SIGNAL( statusMsg( const QString& ) ),
           this, SLOT( slotShowStatusMsg( const QString&  ) ) );

  // launch commandline specified module if any
  // TODO: GUI Option
  activatePluginModule();

}

MainWindow::~MainWindow()
{
  saveSettings();

  QPtrList<KParts::Part> parts = *mPartManager->parts();

  for ( KParts::Part *p = parts.last(); p; p = parts.prev() ) {
    delete p;
    p = 0;
  }

  Prefs::self()->writeConfig();
}

void MainWindow::activePluginModule( const QString &_module )
{
  mActiveModule = _module;
}

void MainWindow::activatePluginModule()
{
  if ( !mActiveModule.isEmpty() )
  {
    PluginList::ConstIterator end = mPlugins.end();
    for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it )
      if ( ( *it )->identifier().contains( mActiveModule ) ) {
        selectPlugin( *it );
        return;
      }
  }
}

void MainWindow::initWidgets()
{
  QHBox *topWidget = new QHBox( this );
  topWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  mTopWidget = topWidget;

  setCentralWidget( mTopWidget );

  mSidePaneType = Prefs::self()->mSidePaneType;

  QHBox *mBox = 0;

  if ( mSidePaneType == Prefs::SidePaneBars ) {
    mSplitter = new QSplitter( mTopWidget );
    mSidePane = new SidePane( this, mSplitter );
    mSidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                           QSizePolicy::Preferred ) );
    mSplitter->setResizeMode( mSidePane, QSplitter::KeepSize );
  } else {
    mSplitter = 0;
    mBox = new QHBox( mTopWidget );
    mSidePane = new IconSidePane( this, mBox );
    mSidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                           QSizePolicy::Preferred ) );
  }

  mSidePane->setActionCollection( actionCollection() );

  connect( mSidePane, SIGNAL( pluginSelected( Kontact::Plugin * ) ),
           SLOT( selectPlugin( Kontact::Plugin * ) ) );

  QVBox *vBox;
  if ( mSplitter ) {
    vBox = new QVBox( mSplitter );
  } else {
    vBox = new QVBox( mBox );
  }

  initHeaderWidget( vBox );
  if ( mSidePaneType != Prefs::SidePaneBars )
    mHeaderFrame->hide();

  vBox->setSpacing( 0 );

  mStack = new QWidgetStack( vBox );

  /* Create a progress dialog and hide it. */
  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(), this );
  progressDialog->hide();

  mLittleProgress = new KPIM::StatusbarProgressWidget( progressDialog, statusBar() );

  mStatusMsgLabel = new KStatusBarLabel( i18n( " Initializing..." ), 1, statusBar() );
  mStatusMsgLabel->setAlignment( AlignLeft | AlignVCenter );

  statusBar()->addWidget( mStatusMsgLabel, 1 , false );
  statusBar()->addWidget( mLittleProgress, 0 , true );
  mLittleProgress->show();
}

void MainWindow::setupActions()
{
  KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
  mNewActions = new KToolBarPopupAction( KGuiItem(i18n( "New" ), ""),
                                         KShortcut(), this, SLOT( slotNewClicked() ),
                                         actionCollection(), "action_new" );

  new KAction( i18n( "Select Components..." ), "configure", 0, this,
               SLOT( slotSelectComponents() ),
               actionCollection(), "settings_select_components" );

  new KAction( i18n( "Configure Kontact..." ), "configure", 0, this, SLOT( slotPreferences() ),
               actionCollection(), "settings_configure_kontact" );

  new KAction( i18n( "&Tip of the Day" ), 0, this, SLOT( slotShowTip() ),
               actionCollection(), "help_tipofday" );
  new KAction( i18n( "&Request Feature..." ), 0, this, SLOT( slotRequestFeature() ),
               actionCollection(), "help_requestfeature" );

}

void MainWindow::initHeaderWidget( QVBox *vBox )
{
  // Initiate the headerWidget
  mHeaderFrame = new QHBox( vBox );
  mHeaderFrame->setSizePolicy( QSizePolicy::MinimumExpanding,
                               QSizePolicy::Maximum );
  mHeaderFrame->setSpacing( 0 );
  mHeaderFrame->setFixedHeight( 22 );

  mHeaderText = new QLabel( mHeaderFrame );
  mHeaderText->setSizePolicy( QSizePolicy::MinimumExpanding,
                              QSizePolicy::Preferred );
  mHeaderText->setPaletteForegroundColor( colorGroup().light() );
  mHeaderText->setPaletteBackgroundColor( colorGroup().dark() );

  mHeaderPixmap = new QLabel( mHeaderFrame );
  mHeaderPixmap->setSizePolicy( QSizePolicy::Maximum,
                                QSizePolicy::Preferred );
  mHeaderPixmap->setAlignment( AlignRight|AlignVCenter );
  mHeaderPixmap->setPaletteBackgroundColor( colorGroup().dark() );

  connect( this, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( setHeaderText( const QString& ) ) );
  connect( this, SIGNAL( iconChanged( const QPixmap& ) ),
           this, SLOT( setHeaderPixmap( const QPixmap& ) ) );

  QFont fnt( mSidePane->font() );
  fnt.setBold( true );
  fnt.setPointSize( mSidePane->font().pointSize() + 3 );
  mHeaderText->setFont( fnt );
}

bool MainWindow::isPluginLoaded( const KPluginInfo * info )
{
  return ( pluginFromInfo( info ) != 0 );
}

Plugin *MainWindow::pluginFromInfo( const KPluginInfo *info )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it )
    if ( ( *it )->identifier() == info->pluginName() )
      return *it;

  return 0;
}

void MainWindow::loadPlugins()
{
  QPtrList<Plugin> plugins;
  QPtrList<KParts::Part> loadDelayed;

  uint i;
  KPluginInfo::List::ConstIterator it;
  for ( it = mPluginInfos.begin(); it != mPluginInfos.end(); ++it ) {
    if ( ! ( *it )->isPluginEnabled() )
      continue;
    if ( isPluginLoaded( *it ) ) {
      Plugin *plugin = pluginFromInfo( *it );
      if ( plugin )
        plugin->configUpdated();
      continue;
    }

    kdDebug(5600) << "Loading Plugin: " << ( *it )->name() << endl;
    Kontact::Plugin *plugin =
      KParts::ComponentFactory::createInstanceFromService<Kontact::Plugin>(
          ( *it )->service(), this );

    if ( !plugin )
      continue;

    plugin->setIdentifier( ( *it )->pluginName() );
    plugin->setTitle( ( *it )->name() );
    plugin->setIcon( ( *it )->icon() );

    QVariant libNameProp = ( *it )->property( "X-KDE-KontactPartLibraryName" );
    QVariant exeNameProp = ( *it )->property( "X-KDE-KontactPartExecutableName" );
    QVariant loadOnStart = ( *it )->property( "X-KDE-KontactPartLoadOnStart" );

    if ( !loadOnStart.isNull() && loadOnStart.toBool() )
      mDelayedPreload.append( plugin );

    kdDebug(5600) << "LIBNAMEPART: " << libNameProp.toString() << endl;

    plugin->setPartLibraryName( libNameProp.toString().utf8() );
    plugin->setExecutableName( exeNameProp.toString() );

    for ( i = 0; i < plugins.count(); ++i ) {
      Plugin *p = plugins.at( i );
      if ( plugin->weight() < p->weight() ) break;
    }

    plugins.insert( i, plugin );
  }

  for ( i = 0; i < plugins.count(); ++ i ) {
    Plugin *plugin = plugins.at( i );

    KAction *action;
    QPtrList<KAction> *actionList = plugin->newActions();

    for ( action = actionList->first(); action; action = actionList->next() ) {
      kdDebug(5600) << "Plugging " << action->name() << endl;
      action->plug( mNewActions->popupMenu() );
    }

    addPlugin( plugin );
  }

  mLastInfoExtension = 0;

  mNewActions->setEnabled( mPlugins.size() != 0 );
}

void MainWindow::unloadPlugins()
{
  KPluginInfo::List::ConstIterator end = mPluginInfos.end();
  KPluginInfo::List::ConstIterator it;
  for ( it = mPluginInfos.begin(); it != end; ++it ) {
    if ( ! ( *it )->isPluginEnabled() )
      removePlugin( *it );
  }
}

bool MainWindow::removePlugin( const KPluginInfo * info )
{
  PluginList::Iterator end = mPlugins.end();
  for ( PluginList::Iterator it = mPlugins.begin(); it != end; ++it )
    if ( ( *it )->identifier() == info->pluginName() ) {
      Plugin *plugin = *it;

      KAction *action;
      QPtrList<KAction> *actionList = plugin->newActions();

      for ( action = actionList->first(); action; action = actionList->next() ) {
        kdDebug(5600) << "Unplugging " << action->name() << endl;
        action->unplug( mNewActions->popupMenu() );
      }

      removeChildClient( plugin );

      if ( mCurrentPlugin == plugin )
        mCurrentPlugin = 0;

      delete plugin; // removes the part automatically
      mPlugins.remove( it );

      return true;
    }

  return false;
}

void MainWindow::addPlugin( Kontact::Plugin *plugin )
{
  kdDebug(5600) << "Added plugin" << endl;

  mPlugins.append( plugin );

  // merge the plugins GUI into the main window
  insertChildClient( plugin );
}

void MainWindow::partLoaded( Kontact::Plugin * /*plugin*/, KPIM::Part *part )
{
  if ( part->widget() )
    mStack->addWidget( part->widget() );

  mPartManager->addPart( part, false );
  // Workaround for KParts misbehavior: addPart calls show!
  part->widget()->hide();
}

void MainWindow::slotActivePartChanged( KParts::Part *part )
{
  if ( !part ) {
    createGUI( 0 );
    return;
  }

  if ( mLastInfoExtension ) {
    disconnect( mLastInfoExtension, SIGNAL( textChanged( const QString& ) ),
                this, SLOT( setHeaderText( const QString& ) ) );
    disconnect( mLastInfoExtension, SIGNAL( iconChanged( const QPixmap& ) ),
                this, SLOT( setHeaderPixmap( const QPixmap& ) ) );
  }

  kdDebug(5600) << "Part activated: " << part << " with stack id. "
      << mStack->id( part->widget() )<< endl;
  QObjectList *l = part->queryList( "KParts::InfoExtension" );
  KParts::InfoExtension *ie = 0;
  if ( l )
    ie = static_cast<KParts::InfoExtension*>( l->first() );
  delete l;

  if ( ie ) {
    connect( ie, SIGNAL( textChanged( const QString& ) ),
             SLOT( setHeaderText( const QString& ) ) );
    connect( ie, SIGNAL( iconChanged( const QPixmap& ) ),
             SLOT( setHeaderPixmap( const QPixmap& ) ) );
  }

  mLastInfoExtension = ie;

  InfoExtData data = mInfoExtCache[ ie ];
  setHeaderPixmap( data.pixmap );
  setHeaderText( data.text );

  createGUI( part );

  statusBar()->clear();
}

void MainWindow::slotNewClicked()
{
  KAction *action = mCurrentPlugin->newActions()->first();
  if ( action ) {
    action->activate();
  } else {
    PluginList::Iterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      action = (*it)->newActions()->first();
      if ( action ) {
        action->activate();
        return;
      }
    }
  }
}

void MainWindow::selectPlugin( Kontact::Plugin *plugin )
{
  if ( !plugin )
    return;

  if ( plugin->isRunningStandalone() ) {
    statusBar()->message( i18n( "Application is running standalone. Foregrounding..." ), 1000 );
    plugin->bringToForeground();
    return;
  }

  KApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if ( mSidePane )
    mSidePane->selectPlugin( plugin );

  KParts::Part *part = plugin->part();

  if ( !part ) {
    KMessageBox::error( this, i18n( "Cannot load part for %1." )
                              .arg( plugin->title() ) );
    KApplication::restoreOverrideCursor();
    return;
  }

  plugin->select();

  mPartManager->setActivePart( part );
  QWidget *view = part->widget();
  Q_ASSERT( view );

  if ( view ) {
    mStack->raiseWidget( view );
    view->show();
    view->setFocus();
    mCurrentPlugin = plugin;
    KAction *action = plugin->newActions()->first();
    setCaption( i18n( "Plugin dependent window title" ,"%1 - Kontact" ).arg( plugin->title() ) );
    if ( action ) {
      mNewActions->setIcon( action->icon() );
      mNewActions->setText( action->text() );
    } else { // we'll use the action of the first plugin which offers one
      PluginList::Iterator it;
      for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
        action = (*it)->newActions()->first();
        if ( action ) {
          mNewActions->setIcon( action->icon() );
          mNewActions->setText( action->text() );
          break;
        }
      }
    }
  }

  QStringList invisibleActions = plugin->invisibleToolbarActions();

  QStringList::ConstIterator it;
  for ( it = invisibleActions.begin(); it != invisibleActions.end(); ++it ) {
    KAction *action = part->actionCollection()->action( (*it).latin1() );
    if ( action )
      action->unplug( toolBar() );
  }

  KApplication::restoreOverrideCursor();
}

void MainWindow::selectPlugin( const QString &pluginName )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it )
    if ( ( *it )->identifier() == pluginName ) {
      selectPlugin( *it );
      return;
    }
}

void MainWindow::loadSettings()
{
  if ( mSplitter )
    mSplitter->setSizes( Prefs::self()->mSidePaneSplitter );

  // Preload Plugins. This _must_ happen before the default part is loaded
  PluginList::ConstIterator it;
  for ( it = mDelayedPreload.begin(); it != mDelayedPreload.end(); ++it )
    selectPlugin( *it );

  selectPlugin( Prefs::self()->mActivePlugin );
}

void MainWindow::saveSettings()
{
  if ( mSplitter )
    Prefs::self()->mSidePaneSplitter = mSplitter->sizes();

  if ( mCurrentPlugin )
    Prefs::self()->mActivePlugin = mCurrentPlugin->identifier();
}

void MainWindow::slotShowTip()
{
  showTip( true );
}

void MainWindow::slotRequestFeature()
{
  if ( kapp )
    kapp->invokeBrowser( "http://kontact.org/shopping" );
}

void MainWindow::showTip(bool force)
{
  QStringList tips;
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    QString file = ( *it )->tipFile();
    if ( !file.isEmpty() )
      tips.append( file );
  }

  KTipDialog::showMultiTip(this, tips, force);
}

void MainWindow::slotQuit()
{
  mReallyClose=true;
  close();
}

void MainWindow::slotPreferences()
{
  static KSettings::Dialog *dlg = 0;
  if( !dlg ) {
    dlg = new KSettings::Dialog( KSettings::Dialog::Configurable, this );
    dlg->addPluginInfos( mPluginInfos );
    connect( dlg, SIGNAL( pluginSelectionChanged() ),
         SLOT( pluginsChanged() ) );
//    connect( dlg, SIGNAL( okClicked() ), SLOT( pluginsChanged() ) );
//    connect( dlg, SIGNAL( applyClicked() ), SLOT( pluginsChanged() ) );
  }

  dlg->show();
}

void MainWindow::slotSelectComponents()
{
  static KSettings::ComponentsDialog *dlg = 0;
  if ( !dlg ) {
    dlg = new KSettings::ComponentsDialog( this );
    dlg->setPluginInfos( mPluginInfos );
    connect( dlg, SIGNAL( okClicked() ), SLOT( pluginsChanged() ) );
    connect( dlg, SIGNAL( applyClicked() ), SLOT( pluginsChanged() ) );
  }

  dlg->show();
}

int MainWindow::startServiceFor( const QString& serviceType,
                                 const QString& constraint,
                                 const QString& preferences,
                                 QString *error, QCString* dcopService,
                                 int flags )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    if ( ( *it )->createDCOPInterface( serviceType ) ) {
      kdDebug(5600) << "found interface for " << serviceType << endl;
      if ( dcopService )
        *dcopService = ( *it )->dcopClient()->appId();
      kdDebug(5600) << "appId=" << ( *it )->dcopClient()->appId() << endl;
      return 0; // success
    }
  }
  kdDebug(5600) <<
    "Didn't find dcop interface, falling back to external process" << endl;
  return KDCOPServiceStarter::startServiceFor( serviceType, constraint,
      preferences, error, dcopService, flags );
}

void MainWindow::setHeaderText( const QString &text )
{
  mInfoExtCache[ mLastInfoExtension ].text = text;
  mHeaderText->setText( text );
}

void MainWindow::setHeaderPixmap( const QPixmap &pixmap )
{
  QPixmap pm( pixmap );

  if ( pm.height() > 22 || pm.width() > 22 ) {
    QImage img;
    img = pixmap;
    pm = img.smoothScale( 22, 22, QImage::ScaleMin );
  }

  mInfoExtCache[ mLastInfoExtension ].pixmap = pm;
  mHeaderPixmap->setPixmap( pm );
}

void MainWindow::pluginsChanged()
{
  unloadPlugins();
  loadPlugins();
  mSidePane->updatePlugins();
}

void MainWindow::updateConfig()
{
  kdDebug( 5600 ) << k_funcinfo << endl;

  saveSettings();

#if 0
  bool sidePaneChanged = ( Prefs::self()->mSidePaneType != mSidePaneType );

  if ( sidePaneChanged ) {
    mSidePaneType = Prefs::self()->mSidePaneType;

    delete mSidePane;

    switch ( mSidePaneType ) {
      case Prefs::SidePaneIcons:
        mSidePane = new IconSidePane( this, mSplitter );
        mHeaderFrame->hide();
        break;
      default:
        kdError() << "Invalid SidePaneType: " << mSidePaneType << endl;
      case Prefs::SidePaneBars:
        mSidePane = new SidePane( this, mSplitter );
        mHeaderFrame->show();
        break;
    }

    mSplitter->setResizeMode( mSidePane, QSplitter::KeepSize );

    mSidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                               QSizePolicy::Preferred ) );

    connect( mSidePane, SIGNAL( pluginSelected( Kontact::Plugin * ) ),
             SLOT( selectPlugin( Kontact::Plugin * ) ) );

    mSplitter->moveToFirst( mSidePane );

    mSidePane->show();
  }

  if ( sidePaneChanged )
    mSidePane->updatePlugins();
#endif

  loadSettings();
}

void MainWindow::showAboutDialog()
{
  KApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if ( !mAboutDialog )
    mAboutDialog = new AboutDialog( this );

  mAboutDialog->show();
  mAboutDialog->raise();
  KApplication::restoreOverrideCursor();
}

void MainWindow::configureShortcuts()
{
  KKeyDialog dialog( true, this );
  dialog.insert( actionCollection() );

  if ( mCurrentPlugin && mCurrentPlugin->part() )
    dialog.insert( mCurrentPlugin->part()->actionCollection() );

  dialog.configure();
}

void MainWindow::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );

  KEditToolbar edit( factory() );
  connect( &edit, SIGNAL( newToolbarConfig() ),
           this, SLOT( slotNewToolbarConfig() ) );
  edit.exec();
}

void MainWindow::slotNewToolbarConfig()
{
  createGUI( mCurrentPlugin->part() );
  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

bool MainWindow::queryClose()
{
  if ( kapp->sessionSaving() || mReallyClose )
    return true;

  bool localClose = true;
  QValueList<Plugin*>::ConstIterator end = mPlugins.end();
  QValueList<Plugin*>::ConstIterator it = mPlugins.begin();
  for ( ; it != end; ++it ) {
    Plugin *plugin = *it;
    if ( !plugin->isRunningStandalone() )
      if ( !plugin->queryClose() )
        localClose = false;
  }

  return localClose;
}


void MainWindow::slotShowStatusMsg( const QString &msg )
{
  if ( !statusBar() || !mStatusMsgLabel ) return;
  int statusWidth = mStatusMsgLabel->width() - fontMetrics().maxWidth();
  QString text = KStringHandler::rPixelSqueeze( " " + msg, fontMetrics(),
                                                statusWidth );
  mStatusMsgLabel->setText( text );
}
#include "mainwindow.moc"
