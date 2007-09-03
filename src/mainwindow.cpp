/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2005 Daniel Molkentin <molkentin@kde.org>
    Copyright (c) 2003-2005 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qcombobox.h>
#include <qdockarea.h>
#include <qguardedptr.h>
#include <qhbox.h>
#include <qimage.h>
#include <qobjectlist.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qwhatsthis.h>

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
#include <krsqueezedtextlabel.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <libkdepim/kfileio.h>
#include <kcursor.h>
#include <krun.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <kstdaccel.h>
#include <kcmultidialog.h>
#include <kipc.h>

#include "aboutdialog.h"
#include "iconsidepane.h"
#include "mainwindow.h"
#include "plugin.h"
#include "prefs.h"
#include "profiledialog.h"
#include "profilemanager.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "broadcaststatus.h"

using namespace Kontact;

class SettingsDialogWrapper : public KSettings::Dialog
{
  public:
    SettingsDialogWrapper( ContentInListView content, QWidget * parent = 0 )
      : KSettings::Dialog( content, parent, 0 )
    {
    }


    void fixButtonLabel( QWidget *widget )
    {
      QObject *object = widget->child( "KJanusWidget::buttonBelowList" );
      QPushButton *button = static_cast<QPushButton*>( object );
      if ( button )
        button->setText( i18n( "Select Components ..." ) );
    }
};

MainWindow::MainWindow()
  : Kontact::Core(), mTopWidget( 0 ), mSplitter( 0 ),
    mCurrentPlugin( 0 ), mAboutDialog( 0 ), mReallyClose( false ), mSyncActionsEnabled( true )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  initGUI();
  initObject();
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

  connect( Kontact::ProfileManager::self(), SIGNAL( profileLoaded( const QString& ) ), 
           this, SLOT( slotLoadProfile( const QString& ) ) );
  connect( Kontact::ProfileManager::self(), SIGNAL( saveToProfileRequested( const QString& ) ), 
           this, SLOT( slotSaveToProfile( const QString& ) ) );
}


void MainWindow::initObject()
{
  KTrader::OfferList offers = KTrader::self()->query(
      QString::fromLatin1( "Kontact/Plugin" ),
      QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  mPluginInfos = KPluginInfo::fromServices( offers, Prefs::self()->config(), "Plugins" );

  KPluginInfo::List::Iterator it;
  for ( it = mPluginInfos.begin(); it != mPluginInfos.end(); ++it ) {
    ( *it )->load();
  }

  // prepare the part manager
  mPartManager = new KParts::PartManager( this );
  connect( mPartManager, SIGNAL( activePartChanged( KParts::Part* ) ),
           this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

  loadPlugins();

  if ( mSidePane ) {
    mSidePane->updatePlugins();
    plugActionList( "navigator_actionlist", mSidePane->actions() );
  }

  KSettings::Dispatcher::self()->registerInstance( instance(), this,
                                                   SLOT( updateConfig() ) );

  loadSettings();

  statusBar()->show();

  showTip( false );

  // done initializing
  slotShowStatusMsg( QString::null );

  connect( KPIM::BroadcastStatus::instance(), SIGNAL( statusMsg( const QString& ) ),
           this, SLOT( slotShowStatusMsg( const QString&  ) ) );

  // launch commandline specified module if any
  activatePluginModule();

  if ( Prefs::lastVersionSeen() == kapp->aboutData()->version() ) {
    selectPlugin( mCurrentPlugin );
  }

  paintAboutScreen( introductionString() );
  Prefs::setLastVersionSeen( kapp->aboutData()->version() );
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

void MainWindow::setActivePluginModule( const QString &module )
{
  mActiveModule = module;
  activatePluginModule();
}

void MainWindow::activatePluginModule()
{
  if ( !mActiveModule.isEmpty() ) {
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
  // includes sidebar and part stack
  mTopWidget = new QHBox( this );
  mTopWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setCentralWidget( mTopWidget );

  QHBox *mBox = 0;
  mSplitter = new QSplitter( mTopWidget );
  mBox = new QHBox( mTopWidget );
  mSidePane = new IconSidePane( this, mSplitter );
  mSidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                         QSizePolicy::Preferred ) );
  // donÄt occupy screen estate on load
  QValueList<int> sizes;
  sizes << 0;
  mSplitter->setSizes(sizes);

  mSidePane->setActionCollection( actionCollection() );

  connect( mSidePane, SIGNAL( pluginSelected( Kontact::Plugin * ) ),
           SLOT( selectPlugin( Kontact::Plugin * ) ) );

  QVBox *vBox;
  if ( mSplitter ) {
    vBox = new QVBox( mSplitter );
  } else {
    vBox = new QVBox( mBox );
  }

  vBox->setSpacing( 0 );

  mPartsStack = new QWidgetStack( vBox );
  initAboutScreen();

  QString loading = i18n( "<h2 style='text-align:center; margin-top: 0px; margin-bottom: 0px'>%1</h2>" )
                    .arg( i18n("Loading Kontact...") );

  paintAboutScreen( loading );

  /* Create a progress dialog and hide it. */
  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(), this );
  progressDialog->hide();

  mLittleProgress = new KPIM::StatusbarProgressWidget( progressDialog, statusBar() );

  mStatusMsgLabel = new KRSqueezedTextLabel( i18n( " Initializing..." ), statusBar() );
  mStatusMsgLabel->setAlignment( AlignLeft | AlignVCenter );

  statusBar()->addWidget( mStatusMsgLabel, 10 , false );
  statusBar()->addWidget( mLittleProgress, 0 , true );
  mLittleProgress->show();
}


void MainWindow::paintAboutScreen( const QString& msg )
{
  QString location = locate( "data", "kontact/about/main.html" );
  QString content = KPIM::kFileToString( location );
  content = content.arg( locate( "data", "libkdepim/about/kde_infopage.css" ) );
  if ( kapp->reverseLayout() )
    content = content.arg( "@import \"%1\";" ).arg( locate( "data", "libkdepim/about/kde_infopage_rtl.css" ) );
  else
    content = content.arg( "" );

  mIntroPart->begin( KURL( location ) );

  QString appName( i18n( "KDE Kontact" ) );
  QString catchPhrase( i18n( "Get Organized!" ) );
  QString quickDescription( i18n( "The KDE Personal Information Management Suite" ) );

  mIntroPart->write( content.arg( QFont().pointSize() + 2 ).arg( appName )
      .arg( catchPhrase ).arg( quickDescription ).arg( msg ) );
  mIntroPart->end();
}

void MainWindow::initAboutScreen()
{
  QHBox *introbox = new QHBox( mPartsStack );
  mPartsStack->addWidget( introbox );
  mPartsStack->raiseWidget( introbox );
  mIntroPart = new KHTMLPart( introbox );
  mIntroPart->widget()->setFocusPolicy( WheelFocus );
  // Let's better be paranoid and disable plugins (it defaults to enabled):
  mIntroPart->setPluginsEnabled( false );
  mIntroPart->setJScriptEnabled( false ); // just make this explicit
  mIntroPart->setJavaEnabled( false );    // just make this explicit
  mIntroPart->setMetaRefreshEnabled( false );
  mIntroPart->setURLCursor( KCursor::handCursor() );
  mIntroPart->view()->setLineWidth( 0 );

  connect( mIntroPart->browserExtension(),
           SIGNAL( openURLRequest( const KURL&, const KParts::URLArgs& ) ),
           SLOT( slotOpenUrl( const KURL& ) ) );

  connect( mIntroPart->browserExtension(),
           SIGNAL( createNewWindow( const KURL&, const KParts::URLArgs& ) ),
           SLOT( slotOpenUrl( const KURL& ) ) );
}

void MainWindow::setupActions()
{
  KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
  mNewActions = new KToolBarPopupAction( KGuiItem( i18n( "New" ), "" ),
                                         KStdAccel::shortcut(KStdAccel::New), this, SLOT( slotNewClicked() ),
                                         actionCollection(), "action_new" );

  KConfig* const cfg = Prefs::self()->config();
  cfg->setGroup( "Kontact Groupware Settings" );
  mSyncActionsEnabled = cfg->readBoolEntry( "GroupwareMailFoldersEnabled", true );

  if ( mSyncActionsEnabled ) {
    mSyncActions = new KToolBarPopupAction( KGuiItem( i18n( "Synchronize" ), "kitchensync" ),
                                            KStdAccel::shortcut(KStdAccel::Reload), this, SLOT( slotSyncClicked() ),
                                            actionCollection(), "action_sync" );
  }
  new KAction( i18n( "Configure Kontact..." ), "configure", 0, this, SLOT( slotPreferences() ),
               actionCollection(), "settings_configure_kontact" );

  new KAction( i18n( "Configure &Profiles..." ), 0, this, SLOT( slotConfigureProfiles() ),
               actionCollection(), "settings_configure_kontact_profiles" );

  new KAction( i18n( "&Kontact Introduction" ), 0, this, SLOT( slotShowIntroduction() ),
               actionCollection(), "help_introduction" );
  new KAction( i18n( "&Tip of the Day" ), 0, this, SLOT( slotShowTip() ),
               actionCollection(), "help_tipofday" );
  new KAction( i18n( "&Request Feature..." ), 0, this, SLOT( slotRequestFeature() ),
               actionCollection(), "help_requestfeature" );
  
  KWidgetAction* spacerAction = new KWidgetAction( new QWidget( this ), "SpacerAction", "", 0, 0, actionCollection(), "navigator_spacer_item" );
  spacerAction->setAutoSized( true );
}

void MainWindow::slotConfigureProfiles()
{
  QGuardedPtr<Kontact::ProfileDialog> dlg = new Kontact::ProfileDialog( this );
  dlg->setModal( true );
  dlg->exec();
  delete dlg;
}

namespace {
    void copyConfigEntry( KConfig* source, KConfig* dest, const QString& group, const QString& key, const QString& defaultValue=QString() )
    {
        source->setGroup( group );
        dest->setGroup( group );
        dest->writeEntry( key, source->readEntry( key, defaultValue ) );
    }
}

void MainWindow::slotSaveToProfile( const QString& id )
{
  const QString path = Kontact::ProfileManager::self()->profileById( id ).saveLocation();
  if ( path.isNull() )
    return;

  KConfig* const cfg = Prefs::self()->config();
  Prefs::self()->writeConfig();
  saveMainWindowSettings( cfg );
  saveSettings();

  KConfig profile( path+"/kontactrc", /*read-only=*/false, /*useglobals=*/false );
  ::copyConfigEntry( cfg, &profile, "MainWindow Toolbar navigatorToolBar", "Hidden", "true" );
  ::copyConfigEntry( cfg, &profile, "View", "SidePaneSplitter" );
  ::copyConfigEntry( cfg, &profile, "Icons", "Theme" );
  
  for ( PluginList::Iterator it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
    if ( !(*it)->isRunningStandalone() ) {
        (*it)->part();
    }
    (*it)->saveToProfile( path );
  }
}

void MainWindow::slotLoadProfile( const QString& id )
{
  const QString path = Kontact::ProfileManager::self()->profileById( id ).saveLocation();
  if ( path.isNull() )
    return;

  KConfig* const cfg = Prefs::self()->config();
  Prefs::self()->writeConfig();
  saveMainWindowSettings( cfg );
  saveSettings();

  const KConfig profile( path+"/kontactrc", /*read-only=*/false, /*useglobals=*/false );
  const QStringList groups = profile.groupList();
  for ( QStringList::ConstIterator it = groups.begin(), end = groups.end(); it != end; ++it )
  {
    cfg->setGroup( *it );
    typedef QMap<QString, QString> StringMap;
    const StringMap entries = profile.entryMap( *it );
    for ( StringMap::ConstIterator it2 = entries.begin(), end = entries.end(); it2 != end; ++it2 )
    {
      if ( it2.data() == "KONTACT_PROFILE_DELETE_KEY" )
        cfg->deleteEntry( it2.key() );
      else
        cfg->writeEntry( it2.key(), it2.data() );
    }
  }

  cfg->sync();
  Prefs::self()->readConfig();
  applyMainWindowSettings( cfg );
  KIconTheme::reconfigure();
  const WId wid = winId();
  KIPC::sendMessage( KIPC::PaletteChanged, wid );
  KIPC::sendMessage( KIPC::FontChanged, wid );
  KIPC::sendMessage( KIPC::StyleChanged, wid );
  KIPC::sendMessage( KIPC::SettingsChanged, wid );
  for ( int i = 0; i < KIcon::LastGroup; ++i )
      KIPC::sendMessage( KIPC::IconChanged, wid, i );

  loadSettings();

  for ( PluginList::Iterator it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
    if ( !(*it)->isRunningStandalone() ) {
        kdDebug() << "Ensure loaded: " << (*it)->identifier() << endl;
        (*it)->part();
    }
    (*it)->loadProfile( path );
  }
}

bool MainWindow::isPluginLoaded( const KPluginInfo *info )
{
  return (pluginFromInfo( info ) != 0);
}

Plugin *MainWindow::pluginFromInfo( const KPluginInfo *info )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it )
    if ( (*it)->identifier() == info->pluginName() )
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
    if ( !(*it)->isPluginEnabled() )
      continue;
    if ( isPluginLoaded( *it ) ) {
      Plugin *plugin = pluginFromInfo( *it );
      if ( plugin )
        plugin->configUpdated();
      continue;
    }

    kdDebug(5600) << "Loading Plugin: " << (*it)->name() << endl;
    Kontact::Plugin *plugin =
      KParts::ComponentFactory::createInstanceFromService<Kontact::Plugin>(
          (*it)->service(), this );

    if ( !plugin )
      continue;

    plugin->setIdentifier( (*it)->pluginName() );
    plugin->setTitle( (*it)->name() );
    plugin->setIcon( (*it)->icon() );

    QVariant libNameProp = (*it)->property( "X-KDE-KontactPartLibraryName" );
    QVariant exeNameProp = (*it)->property( "X-KDE-KontactPartExecutableName" );
    QVariant loadOnStart = (*it)->property( "X-KDE-KontactPartLoadOnStart" );
    QVariant hasPartProp = (*it)->property( "X-KDE-KontactPluginHasPart" );

    if ( !loadOnStart.isNull() && loadOnStart.toBool() )
      mDelayedPreload.append( plugin );

    kdDebug(5600) << "LIBNAMEPART: " << libNameProp.toString() << endl;

    plugin->setPartLibraryName( libNameProp.toString().utf8() );
    plugin->setExecutableName( exeNameProp.toString() );
    if ( hasPartProp.isValid() )
      plugin->setShowInSideBar( hasPartProp.toBool() );

    for ( i = 0; i < plugins.count(); ++i ) {
      Plugin *p = plugins.at( i );
      if ( plugin->weight() < p->weight() )
        break;
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

    if ( mSyncActionsEnabled ) {
      actionList = plugin->syncActions();
      for ( action = actionList->first(); action; action = actionList->next() ) {
        kdDebug(5600) << "Plugging " << action->name() << endl;
        action->plug( mSyncActions->popupMenu() );
      }
    }
    addPlugin( plugin );
  }

  mNewActions->setEnabled( mPlugins.size() != 0 );
  if ( mSyncActionsEnabled )
    mSyncActions->setEnabled( mPlugins.size() != 0 );
}

void MainWindow::unloadPlugins()
{
  KPluginInfo::List::ConstIterator end = mPluginInfos.end();
  KPluginInfo::List::ConstIterator it;
  for ( it = mPluginInfos.begin(); it != end; ++it ) {
    if ( !(*it)->isPluginEnabled() )
      removePlugin( *it );
  }
}

bool MainWindow::removePlugin( const KPluginInfo *info )
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

      if ( mSyncActionsEnabled ) {
        actionList = plugin->syncActions();
        for ( action = actionList->first(); action; action = actionList->next() ) {
          kdDebug(5600) << "Unplugging " << action->name() << endl;
          action->unplug( mSyncActions->popupMenu() );
        }
      }
      removeChildClient( plugin );

      if ( mCurrentPlugin == plugin )
        mCurrentPlugin = 0;

      delete plugin; // removes the part automatically
      mPlugins.remove( it );

      if ( mCurrentPlugin == 0 ) {
        PluginList::Iterator it;
        for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
          if ( (*it)->showInSideBar() ) {
            selectPlugin( *it );
            return true;
          }
        }
      }

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

void MainWindow::partLoaded( Kontact::Plugin*, KParts::ReadOnlyPart *part )
{
  // See if we have this part already (e.g. due to two plugins sharing it)
  if ( mPartsStack->id( part->widget() ) != -1 )
    return;

  mPartsStack->addWidget( part->widget() );

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

  kdDebug(5600) << "Part activated: " << part << " with stack id. "
      << mPartsStack->id( part->widget() )<< endl;

  //createGUI( part ); // moved to selectPlugin()

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

void MainWindow::slotSyncClicked()
{
  KAction *action = mCurrentPlugin->syncActions()->first();
  if ( action ) {
    action->activate();
  } else {
    PluginList::Iterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      action = (*it)->syncActions()->first();
      if ( action ) {
        action->activate();
        return;
      }
    }
  }
}

KToolBar* Kontact::MainWindow::findToolBar(const char* name)
{
  // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
  return static_cast<KToolBar *>(child(name, "KToolBar"));
}

void MainWindow::selectPlugin( Kontact::Plugin *plugin )
{
  if ( !plugin )
    return;

  if ( plugin->isRunningStandalone() ) {
    statusBar()->message( i18n( "Application is running standalone. Foregrounding..." ), 1000 );
    mSidePane->indicateForegrunding( plugin );
    plugin->bringToForeground();
    return;
  }

  KApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  KParts::Part *part = plugin->part();

  if ( !part ) {
    KApplication::restoreOverrideCursor();
    KMessageBox::error( this, i18n( "Cannot load part for %1." )
                              .arg( plugin->title() )
                        + "\n" + lastErrorMessage() );
    plugin->setDisabled( true );
    mSidePane->updatePlugins();
    return;
  }

  // store old focus widget
  QWidget *focusWidget = kapp->focusWidget();
  if ( mCurrentPlugin && focusWidget ) {
    // save the focus widget only when it belongs to the activated part
    QWidget *parent = focusWidget->parentWidget();
    while ( parent ) {
      if ( parent == mCurrentPlugin->part()->widget() )
        mFocusWidgets.insert( mCurrentPlugin->identifier(), QGuardedPtr<QWidget>( focusWidget ) );

      parent = parent->parentWidget();
    }
  }

  if ( mSidePane )
    mSidePane->selectPlugin( plugin );

  plugin->select();

  mPartManager->setActivePart( part );
  QWidget *view = part->widget();
  Q_ASSERT( view );

  if ( view ) {
    mPartsStack->raiseWidget( view );
    view->show();

    if ( mFocusWidgets.contains( plugin->identifier() ) ) {
      focusWidget = mFocusWidgets[ plugin->identifier() ];
      if ( focusWidget )
        focusWidget->setFocus();
    } else
      view->setFocus();

    mCurrentPlugin = plugin;
    KAction *newAction = plugin->newActions()->first();
    KAction *syncAction = plugin->syncActions()->first();

    createGUI( plugin->part() );

    KToolBar* navigatorToolBar = findToolBar( "navigatorToolBar" );
    // Let the navigator toolbar be always the last one, if it's in the top dockwindow
    if ( navigatorToolBar && !navigatorToolBar->isHidden() &&
         navigatorToolBar->barPos() == KToolBar::Top ) {
      topDock()->moveDockWindow( navigatorToolBar, -1 );
    }

    setCaption( i18n( "Plugin dependent window title" ,"%1 - Kontact" ).arg( plugin->title() ) );

    if ( newAction ) {
      mNewActions->setIcon( newAction->icon() );
      mNewActions->setText( newAction->text() );
    } else { // we'll use the action of the first plugin which offers one
      PluginList::Iterator it;
      for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
        newAction = (*it)->newActions()->first();
        if ( newAction ) {
          mNewActions->setIcon( newAction->icon() );
          mNewActions->setText( newAction->text() );
          break;
        }
      }
    }
    if ( mSyncActionsEnabled ) {
      if ( syncAction ) {
        mSyncActions->setIcon( syncAction->icon() );
        mSyncActions->setText( syncAction->text() );
      } else { // we'll use the action of the first plugin which offers one
        PluginList::Iterator it;
        for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
          syncAction = (*it)->syncActions()->first();
          if ( syncAction ) {
            mSyncActions->setIcon( syncAction->icon() );
            mSyncActions->setText( syncAction->text() );
            break;
          }
        }
      }
    }
  }
  QStringList invisibleActions = plugin->invisibleToolbarActions();

  QStringList::ConstIterator it;
  for ( it = invisibleActions.begin(); it != invisibleActions.end(); ++it ) {
    KAction *action = part->actionCollection()->action( (*it).latin1() );
    if ( action ) {
      QPtrListIterator<KToolBar> it(  toolBarIterator() );
      for (  ; it.current() ; ++it ) {
        action->unplug( it.current() );
      }
    }
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

void MainWindow::slotShowIntroduction()
{
  mPartsStack->raiseWidget( 0 ); // ###
}

void MainWindow::showTip( bool force )
{
  QStringList tips;
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    QString file = (*it)->tipFile();
    if ( !file.isEmpty() )
      tips.append( file );
  }

  KTipDialog::showMultiTip( this, tips, force );
}

void MainWindow::slotQuit()
{
  mReallyClose = true;
  close();
}

void MainWindow::slotPreferences()
{
  static SettingsDialogWrapper *dlg = 0;
  if ( !dlg ) {
    // do not show settings of components running standalone
    QValueList<KPluginInfo*> filteredPlugins = mPluginInfos;
    PluginList::ConstIterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it )
      if ( (*it)->isRunningStandalone() ) {
        QValueList<KPluginInfo*>::ConstIterator infoIt;
        for ( infoIt = filteredPlugins.begin(); infoIt != filteredPlugins.end(); ++infoIt ) {
          if ( (*infoIt)->pluginName() == (*it)->identifier() ) {
            filteredPlugins.remove( *infoIt );
            break;
          }
        }
      }
    dlg = new SettingsDialogWrapper( KSettings::Dialog::Configurable, this );
    dlg->addPluginInfos( filteredPlugins );
    connect( dlg, SIGNAL( pluginSelectionChanged() ),
             SLOT( pluginsChanged() ) );
  }

  dlg->show();
  dlg->fixButtonLabel( this );
}

int MainWindow::startServiceFor( const QString& serviceType,
                                 const QString& constraint,
                                 const QString& preferences,
                                 QString *error, QCString* dcopService,
                                 int flags )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    if ( (*it)->createDCOPInterface( serviceType ) ) {
      kdDebug(5600) << "found interface for " << serviceType << endl;
      if ( dcopService )
        *dcopService = (*it)->dcopClient()->appId();
      kdDebug(5600) << "appId=" << (*it)->dcopClient()->appId() << endl;
      return 0; // success
    }
  }

  kdDebug(5600) <<
    "Didn't find dcop interface, falling back to external process" << endl;

  return KDCOPServiceStarter::startServiceFor( serviceType, constraint,
      preferences, error, dcopService, flags );
}

void MainWindow::pluginsChanged()
{
  unplugActionList( "navigator_actionlist" );
  unloadPlugins();
  loadPlugins();
  mSidePane->updatePlugins();
  plugActionList( "navigator_actionlist", mSidePane->actions() );
}

void MainWindow::updateConfig()
{
  kdDebug( 5600 ) << k_funcinfo << endl;

  saveSettings();
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
  if ( mCurrentPlugin && mCurrentPlugin->part() )
    createGUI( mCurrentPlugin->part() );
  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void MainWindow::slotOpenUrl( const KURL &url )
{
  if ( url.protocol() == "exec" ) {
    if ( url.path() == "/switch" ) {
      selectPlugin( mCurrentPlugin );
    }
    if ( url.path() == "/gwwizard" ) {
      KRun::runCommand( "groupwarewizard" );
      slotQuit();
    }
  } else
    new KRun( url, this );
}

void MainWindow::readProperties( KConfig *config )
{
  Core::readProperties( config );

  QStringList activePlugins = config->readListEntry( "ActivePlugins" );
  QValueList<Plugin*>::ConstIterator it = mPlugins.begin();
  QValueList<Plugin*>::ConstIterator end = mPlugins.end();
  for ( ; it != end; ++it ) {
    Plugin *plugin = *it;
    if ( !plugin->isRunningStandalone() ) {
      QStringList::ConstIterator activePlugin = activePlugins.find( plugin->identifier() );
      if ( activePlugin != activePlugins.end() ) {
        plugin->readProperties( config );
      }
    }
  }
}

void MainWindow::saveProperties( KConfig *config )
{
  Core::saveProperties( config );

  QStringList activePlugins;

  KPluginInfo::List::Iterator it = mPluginInfos.begin();
  KPluginInfo::List::Iterator end = mPluginInfos.end();
  for ( ; it != end; ++it ) {
    KPluginInfo *info = *it;
    if ( info->isPluginEnabled() ) {
      Plugin *plugin = pluginFromInfo( info );
      if ( plugin ) {
        activePlugins.append( plugin->identifier() );
        plugin->saveProperties( config );
      }
    }
  }

  config->writeEntry( "ActivePlugins", activePlugins );
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
  if ( !statusBar() || !mStatusMsgLabel )
     return;

  mStatusMsgLabel->setText( msg );
}

QString MainWindow::introductionString()
{
  KIconLoader *iconloader = KGlobal::iconLoader();
  int iconSize = iconloader->currentSize( KIcon::Desktop );

  QString handbook_icon_path = iconloader->iconPath( "contents2",  KIcon::Desktop );
  QString html_icon_path = iconloader->iconPath( "html",  KIcon::Desktop );
  QString wizard_icon_path = iconloader->iconPath( "wizard",  KIcon::Desktop );

  QString info = i18n( "<h2 style='text-align:center; margin-top: 0px;'>Welcome to Kontact %1</h2>"
      "<p>%1</p>"
      "<table align=\"center\">"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "<tr><td><a href=\"%1\"><img width=\"%1\" height=\"%1\" src=\"%1\" /></a></td>"
      "<td><a href=\"%1\">%1</a><br><span id=\"subtext\"><nobr>%1</td></tr>"
      "</table>"
      "<p style=\"margin-bottom: 0px\"> <a href=\"%1\">Skip this introduction</a></p>" )
      .arg( kapp->aboutData()->version() )
      .arg( i18n( "Kontact handles your e-mail, addressbook, calendar, to-do list and more." ) )
      .arg( "help:/kontact" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( handbook_icon_path )
      .arg( "help:/kontact" )
      .arg( i18n( "Read Manual" ) )
      .arg( i18n( "Learn more about Kontact and its components" ) )
      .arg( "http://kontact.org" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( html_icon_path )
      .arg( "http://kontact.org" )
      .arg( i18n( "Visit Kontact Website" ) )
      .arg( i18n( "Access online resources and tutorials" ) )
      .arg( "exec:/gwwizard" )
      .arg( iconSize )
      .arg( iconSize )
      .arg( wizard_icon_path )
      .arg( "exec:/gwwizard" )
      .arg( i18n( "Configure Kontact as Groupware Client" ) )
      .arg( i18n( "Prepare Kontact for use in corporate networks" ) )
      .arg( "exec:/switch" );
  return info;
}

#include "mainwindow.moc"
