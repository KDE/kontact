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

#include <QComboBox>
#include <QCursor>
#include <khbox.h>
#include <QImage>
#include <QObject>
#include <QPushButton>
#include <QSplitter>
#include <QTimer>
#include <QList>
#include <QLayout>

#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kguiitem.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <ktoolbar.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kplugininfo.h>
#include <kmenu.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <ktip.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstringhandler.h>
#include <ksqueezedtextlabel.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <libkdepim/kfileio.h>
#include <krun.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <kstandardshortcut.h>
#include <ktoolinvocation.h>
#include <ktoolbarpopupaction.h>
#include <kvbox.h>
#include <kicon.h>

#include "aboutdialog.h"
#include "iconsidepane.h"
#include "mainwindow.h"
#include "plugin.h"
#include "prefs.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "broadcaststatus.h"

using namespace Kontact;

MainWindow::MainWindow()
  : Kontact::Core(), mTopWidget( 0 ), mSplitter( 0 ),
    mCurrentPlugin( 0 ), mAboutDialog( 0 ), mReallyClose( false )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

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

  KStandardAction::keyBindings( this, SLOT( configureShortcuts() ), actionCollection() );
  KStandardAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
  setXMLFile( "kontactui.rc" );

  setStandardToolBarMenuEnabled( true );

  createGUI( 0 );

  resize( 700, 520 ); // initial size to prevent a scrollbar in sidepane
  setAutoSaveSettings();
}


void MainWindow::initObject()
{
  KService::List offers = KServiceTypeTrader::self()->query(
      QString::fromLatin1( "Kontact/Plugin" ),
      QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  mPluginInfos = KPluginInfo::fromServices( offers, KConfigGroup( Prefs::self()->config(), "Plugins" ) );

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

  KSettings::Dispatcher::registerComponent( componentData(), this, SLOT( updateConfig() ) );

  loadSettings();

  statusBar()->show();

  showTip( false );

  // done initializing
  slotShowStatusMsg( QString::null );

  connect( KPIM::BroadcastStatus::instance(), SIGNAL( statusMsg( const QString& ) ),
           this, SLOT( slotShowStatusMsg( const QString&  ) ) );

  // launch commandline specified module if any
  activatePluginModule();

  if ( Prefs::lastVersionSeen() == KGlobal::mainComponent().aboutData()->version() ) {
    selectPlugin( mCurrentPlugin );
  }

//  paintAboutScreen( introductionString() );
  Prefs::setLastVersionSeen( KGlobal::mainComponent().aboutData()->version() );
}

MainWindow::~MainWindow()
{
  saveSettings();

  QList<KParts::Part*> parts = mPartManager->parts();

  Q_FOREACH( KParts::Part *p, parts ) {
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
  mTopWidget = new KHBox( this );
  mTopWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setCentralWidget( mTopWidget );

  KHBox *mBox = 0;
  mSplitter = new QSplitter( mTopWidget );
  mBox = new KHBox( mTopWidget );
  mSidePane = new IconSidePane( this, mSplitter );
  mSidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                         QSizePolicy::Preferred ) );
/*
  // don't occupy screen estate on load

  QList<int> sizes;
  sizes << 0;
  mSplitter->setSizes(sizes);
*/
  mSidePane->setActionCollection( actionCollection() );

  connect( mSidePane, SIGNAL( pluginSelected( Kontact::Plugin * ) ),
           SLOT( selectPlugin( Kontact::Plugin * ) ) );

  KVBox *vBox = new KVBox( mSplitter );

  vBox->setSpacing( 0 );

  mPartsStack = new QStackedWidget( vBox );
  mPartsStack->layout()->setSpacing(0);


/*
  initAboutScreen();

  QString loading = i18n( "<h2 style='text-align:center; margin-top: 0px; margin-bottom: 0px'>%1</h2>" )
                    .arg( i18n("Loading Kontact...") );

  paintAboutScreen( loading );
*/
  /* Create a progress dialog and hide it. */
  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(), this );
  progressDialog->hide();

  mLittleProgress = new KPIM::StatusbarProgressWidget( progressDialog, statusBar() );

  mStatusMsgLabel = new KSqueezedTextLabel( i18n( " Initializing..." ), statusBar() );
  mStatusMsgLabel->setTextElideMode( Qt::ElideRight );
  mStatusMsgLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

  statusBar()->addWidget( mStatusMsgLabel, 10 );
  statusBar()->addPermanentWidget( mLittleProgress, 0 );
  mLittleProgress->show();
}


void MainWindow::paintAboutScreen( const QString& msg )
{
  QString location = KStandardDirs::locate( "data", "kontact/about/main.html" );
  QString content = KPIM::kFileToByteArray( location );
  content = content.arg( KStandardDirs::locate( "data", "libkdepim/about/kde_infopage.css" ) );
  if ( QApplication::isRightToLeft() )
    content = content.arg( "@import \"%1\";" ).arg( KStandardDirs::locate( "data", "libkdepim/about/kde_infopage_rtl.css" ) );
  else
    content = content.arg( "" );

  mIntroPart->begin( KUrl( location ) );

  QString appName( i18n( "KDE Kontact" ) );
  QString catchPhrase( i18n( "Get Organized!" ) );
  QString quickDescription( i18n( "The KDE Personal Information Management Suite" ) );

  mIntroPart->write( content.arg( QFont().pointSize() + 2 ).arg( appName )
      .arg( catchPhrase ).arg( quickDescription ).arg( msg ) );
  mIntroPart->end();
}

void MainWindow::initAboutScreen()
{
  KHBox *introbox = new KHBox( mPartsStack );
  mPartsStack->addWidget( introbox );
  mPartsStack->setCurrentWidget( introbox );
  mIntroPart = new KHTMLPart( introbox );
  mIntroPart->widget()->setFocusPolicy( Qt::WheelFocus );
  // Let's better be paranoid and disable plugins (it defaults to enabled):
  mIntroPart->setPluginsEnabled( false );
  mIntroPart->setJScriptEnabled( false ); // just make this explicit
  mIntroPart->setJavaEnabled( false );    // just make this explicit
  mIntroPart->setMetaRefreshEnabled( false );
  mIntroPart->setURLCursor( QCursor( Qt::PointingHandCursor ) );
  mIntroPart->view()->setLineWidth( 0 );

  connect( mIntroPart->browserExtension(),
           SIGNAL( openUrlRequest( const KUrl&, const KParts::URLArgs& ) ),
           SLOT( slotOpenUrl( const KUrl& ) ) );

  connect( mIntroPart->browserExtension(),
           SIGNAL( createNewWindow( const KUrl&, const KParts::URLArgs& ) ),
           SLOT( slotOpenUrl( const KUrl& ) ) );
}

void MainWindow::setupActions()
{
    actionCollection()->addAction(KStandardAction::Quit, this, SLOT( slotQuit() ));

    mNewActions = new KToolBarPopupAction( KIcon(""), i18n( "New" ), this);
    actionCollection()->addAction("action_new", mNewActions);
    mNewActions->setShortcut(KShortcut(KStandardShortcut::New));
    connect(mNewActions, SIGNAL(triggered(bool)), this, SLOT( slotNewClicked() ));

    KAction *action  = new KAction(KIcon("configure"), i18n("Configure Kontact..."), this);
    actionCollection()->addAction("settings_configure_kontact", action );
  connect(action, SIGNAL(triggered(bool)), SLOT( slotPreferences() ));

    action  = new KAction(i18n("&Kontact Introduction"), this);
    actionCollection()->addAction("help_introduction", action );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotShowIntroduction() ));
    action  = new KAction(i18n("&Tip of the Day"), this);
    actionCollection()->addAction("help_tipofday", action );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotShowTip() ));
    action  = new KAction(i18n("&Request Feature..."), this);
    actionCollection()->addAction("help_requestfeature", action );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotRequestFeature() ));

  QWidgetAction* spacerAction = new QWidgetAction(this);
  QWidget* stretchWidget = new QWidget( this );
  stretchWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred ) );
  spacerAction->setDefaultWidget( stretchWidget );
  actionCollection()->addAction("navigator_spacer_item", spacerAction);
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
  QList<Plugin*> plugins;
//  QList<KParts::Part*> loadDelayed;

  int i;
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

    kDebug(5600) << "Loading Plugin: " << (*it)->name() << endl;
    Kontact::Plugin *plugin =
      KService::createInstance<Kontact::Plugin>(
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

    kDebug(5600) << "LIBNAMEPART: " << libNameProp.toString() << endl;

    plugin->setPartLibraryName( libNameProp.toString().toUtf8() );
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

    const QList<KAction*> *actionList = plugin->newActions();
    QList<KAction*>::const_iterator listIt;

    for ( listIt = actionList->begin(); listIt != actionList->end(); ++listIt ) {
      kDebug(5600) << "Plugging " << (*listIt)->objectName() << endl;
      mNewActions->popupMenu()->addAction( (*listIt) );
    }

    addPlugin( plugin );
  }

  mNewActions->setEnabled( mPlugins.size() != 0 );
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

      const QList<KAction*> *actionList = plugin->newActions();
      QList<KAction*>::const_iterator listIt;

      for ( listIt = actionList->begin(); listIt != actionList->end(); ++listIt ) {
        kDebug(5600) << "Unplugging " << (*listIt)->objectName() << endl;
        mNewActions->popupMenu()->removeAction( *listIt );
      }

      removeChildClient( plugin );

      if ( mCurrentPlugin == plugin )
        mCurrentPlugin = 0;

      delete plugin; // removes the part automatically
      mPlugins.erase( it );

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
  kDebug(5600) << "Added plugin" << endl;

  mPlugins.append( plugin );

  // merge the plugins GUI into the main window
  insertChildClient( plugin );
}

void MainWindow::partLoaded( Kontact::Plugin*, KParts::ReadOnlyPart *part )
{
  // See if we have this part already (e.g. due to two plugins sharing it)
  if ( mPartsStack->indexOf( part->widget() ) != -1 )
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

  kDebug(5600) << "Part activated: " << part << " with stack id. "
      << mPartsStack->indexOf( part->widget() )<< endl;

  //createGUI( part ); // moved to selectPlugin()

  statusBar()->clearMessage();
}

void MainWindow::slotNewClicked()
{
  QAction *action = mCurrentPlugin->newActions()->first();
  if ( action ) {
    action->trigger();
  } else {
    PluginList::Iterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      action = (*it)->newActions()->first();
      if ( action ) {
        action->trigger();
        return;
      }
    }
  }
}

KToolBar* Kontact::MainWindow::findToolBar(const char* name)
{
  // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
  return findChild<KToolBar *>(name);
}

void MainWindow::selectPlugin( Kontact::Plugin *plugin )
{
  if ( !plugin )
    return;

  if ( plugin->isRunningStandalone() ) {
    statusBar()->showMessage( i18n( "Application is running standalone. Foregrounding..." ), 1000 );
    mSidePane->indicateForegrunding( plugin );
    plugin->bringToForeground();
    return;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  KParts::Part *part = plugin->part();

  if ( !part ) {
    QApplication::restoreOverrideCursor();
    KMessageBox::error( this, i18n( "Cannot load part for %1." ,
                                plugin->title() )
                        + '\n' + lastErrorMessage() );
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
        mFocusWidgets.insert( mCurrentPlugin->identifier(), QPointer<QWidget>( focusWidget ) );

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
    mPartsStack->setCurrentWidget( view );
    view->show();

    if ( mFocusWidgets.contains( plugin->identifier() ) ) {
      focusWidget = mFocusWidgets[ plugin->identifier() ];
      if ( focusWidget )
        focusWidget->setFocus();
    } else
      view->setFocus();

    mCurrentPlugin = plugin;
    QAction *action = 0;
    if (plugin->newActions()->count() > 0)
      action = plugin->newActions()->first();

    createGUI( plugin->part() );

    KToolBar* navigatorToolBar = findToolBar( "navigatorToolBar" );
    // Let the navigator toolbar be always the last one, if it's in the top dockwindow
    if ( navigatorToolBar && !navigatorToolBar->isHidden() &&
         toolBarArea( navigatorToolBar ) == Qt::TopToolBarArea ) {
      addToolBar( Qt::TopToolBarArea, navigatorToolBar );
    }

    setCaption( i18nc( "Plugin dependent window title" ,"%1 - Kontact", plugin->title() ) );

    if ( action ) {
      static_cast<QAction*>( mNewActions )->setIcon( action->icon() );
      mNewActions->setText( action->text() );
    } else { // we'll use the action of the first plugin which offers one
      PluginList::Iterator it;
      for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
        if ((*it)->newActions()->count() > 0)
          action = (*it)->newActions()->first();
        if ( action ) {
          static_cast<QAction*>( mNewActions )->setIcon( action->icon() );
          mNewActions->setText( action->text() );
          break;
        }
      }
    }
  }

  QStringList invisibleActions = plugin->invisibleToolbarActions();

  QStringList::ConstIterator it;
  for ( it = invisibleActions.begin(); it != invisibleActions.end(); ++it ) {
    QAction *action = part->actionCollection()->action( (*it) );
    if ( action ) {
      QList<KToolBar*> toolbars = toolBars();
      for( QList<KToolBar*>::Iterator it = toolbars.begin(); it != toolbars.end(); ++it ) {
        ( *it )->removeAction( action );
      }
    }
  }

  QApplication::restoreOverrideCursor();
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
  KToolInvocation::invokeBrowser( "http://kontact.org/shopping" );
}

void MainWindow::slotShowIntroduction()
{
  mPartsStack->setCurrentWidget( 0 ); // ###
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
  static KSettings::Dialog *dlg = 0;
  if ( !dlg ) {
    dlg = new KSettings::Dialog( this );
    dlg->setAllowComponentSelection( true );

    // do not show settings of components running standalone
    KPluginInfo::List filteredPlugins = mPluginInfos;
    PluginList::ConstIterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it )
      if ( (*it)->isRunningStandalone() ) {
        KPluginInfo::List::ConstIterator infoIt;
        for ( infoIt = filteredPlugins.begin(); infoIt != filteredPlugins.end(); ++infoIt ) {
          if ( (*infoIt)->pluginName() == (*it)->identifier() ) {
            filteredPlugins.removeAll( *infoIt );
            break;
          }
        }
      }

    dlg->addPluginInfos( filteredPlugins );
    connect( dlg, SIGNAL( pluginSelectionChanged() ),
             SLOT( pluginsChanged() ) );
  }

  dlg->show();
}

int MainWindow::startServiceFor( const QString& serviceType,
                                 const QString& constraint,
                                 QString *error, QString* dbusService,
                                 int flags )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
   if ( (*it)->createDBUSInterface( serviceType ) ) {
	    kDebug(5600) << "found interface for " << serviceType << endl;
	    if( dbusService )
               *dbusService = (*it)->registerClient();
            //kDebug(5600) << "appId=" << (*it)->dcopClient()->appId() << endl;
            return 0; // success
    }
  }

  kDebug(5600) <<
    "Didn't find dbus interface, falling back to external process" << endl;
  return KDBusServiceStarter::startServiceFor( serviceType, constraint,
      error, dbusService, flags );
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
  kDebug( 5600 ) << k_funcinfo << endl;

  saveSettings();
  loadSettings();
}

void MainWindow::showAboutDialog()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if ( !mAboutDialog )
    mAboutDialog = new AboutDialog( this );

  mAboutDialog->show();
  mAboutDialog->raise();
  QApplication::restoreOverrideCursor();
}

void MainWindow::configureShortcuts()
{
  KShortcutsDialog dialog( KShortcutsEditor::AllActions ,KShortcutsEditor::LetterShortcutsAllowed, this );
  dialog.addCollection( actionCollection() );

  if ( mCurrentPlugin && mCurrentPlugin->part() )
    dialog.addCollection( mCurrentPlugin->part()->actionCollection() );

  dialog.configure();
}

void MainWindow::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );

  KEditToolBar edit( factory() );
  connect( &edit, SIGNAL( newToolbarConfig() ),
           this, SLOT( slotNewToolbarConfig() ) );
  edit.exec();
}

void MainWindow::slotNewToolbarConfig()
{
  if ( mCurrentPlugin && mCurrentPlugin->part() )
    createGUI( mCurrentPlugin->part() );
  applyMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );
}

void MainWindow::slotOpenUrl( const KUrl &url )
{
  if ( url.protocol() == "exec" ) {
    if ( url.path() == "/switch" ) {
      selectPlugin( mCurrentPlugin );
    }
    if ( url.path() == "/gwwizard" ) {
      KRun::runCommand( "groupwarewizard", this );
      slotQuit();
    }
  } else
    new KRun( url, this );
}

void MainWindow::readProperties( const KConfigGroup &config )
{
  Core::readProperties( config );

  QStringList activePlugins = config.readEntry( "ActivePlugins",QStringList() );
  QList<Plugin*>::ConstIterator it = mPlugins.begin();
  QList<Plugin*>::ConstIterator end = mPlugins.end();
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

void MainWindow::saveProperties( KConfigGroup &config )
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

  config.writeEntry( "ActivePlugins", activePlugins );
}

bool MainWindow::queryClose()
{
  if ( kapp->sessionSaving() || mReallyClose )
    return true;

  bool localClose = true;
  QList<Plugin*>::ConstIterator end = mPlugins.end();
  QList<Plugin*>::ConstIterator it = mPlugins.begin();
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
  KIconLoader *iconloader = KIconLoader::global();
  int iconSize = iconloader->currentSize( K3Icon::Desktop );

  QString handbook_icon_path = iconloader->iconPath( "contents2",  K3Icon::Desktop );
  QString html_icon_path = iconloader->iconPath( "html",  K3Icon::Desktop );
  QString wizard_icon_path = iconloader->iconPath( "wizard",  K3Icon::Desktop );

  QString info = ki18n( "<h2 style='text-align:center; margin-top: 0px;'>Welcome to Kontact %1</h2>"
      "<p>%2</p>"
      "<table align=\"center\">"
      "<tr><td><a href=\"%3\"><img width=\"%4\" height=\"%5\" src=\"%6\" /></a></td>"
      "<td><a href=\"%7\">%8</a><br><span id=\"subtext\"><nobr>%9</td></tr>"
      "<tr><td><a href=\"%10\"><img width=\"%11\" height=\"%12\" src=\"%13\" /></a></td>"
      "<td><a href=\"%14\">%15</a><br><span id=\"subtext\"><nobr>%16</td></tr>"
      "<tr><td><a href=\"%17\"><img width=\"%18\" height=\"%19\" src=\"%20\" /></a></td>"
      "<td><a href=\"%21\">%22</a><br><span id=\"subtext\"><nobr>%23</td></tr>"
      "</table>"
      "<p style=\"margin-bottom: 0px\"> <a href=\"%24\">Skip this introduction</a></p>" )
      .subs( KGlobal::mainComponent().aboutData()->version() )
      .subs( i18n( "Kontact handles your e-mail, addressbook, calendar, to-do list and more." ) )
      .subs( "help:/kontact" )
      .subs( iconSize )
      .subs( iconSize )
      .subs( handbook_icon_path )
      .subs( "help:/kontact" )
      .subs( i18n( "Read Manual" ) )
      .subs( i18n( "Learn more about Kontact and its components" ) )
      .subs( "http://kontact.org" )
      .subs( iconSize )
      .subs( iconSize )
      .subs( html_icon_path )
      .subs( "http://kontact.org" )
      .subs( i18n( "Visit Kontact Website" ) )
      .subs( i18n( "Access online resources and tutorials" ) )
      .subs( "exec:/gwwizard" )
      .subs( iconSize )
      .subs( iconSize )
      .subs( wizard_icon_path )
      .subs( "exec:/gwwizard" )
      .subs( i18n( "Configure Kontact as Groupware Client" ) )
      .subs( i18n( "Prepare Kontact for use in corporate networks" ) )
      .subs( "exec:/switch" )
      .toString();
  return info;
}

#include "mainwindow.moc"
