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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"
#include "aboutdialog.h"
#include "iconsidepane.h"
#include "plugin.h"
#include "prefs.h"
#include "profiledialog.h"
#include "profilemanager.h"
#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "broadcaststatus.h"

#include <kpimutils/kfileio.h>

#include <kparts/componentfactory.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdbusservicestarter.h>
#include <kedittoolbar.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <ktoolbar.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kmenu.h>
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
#include <krun.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <kstandardshortcut.h>
#include <ktoolinvocation.h>
#include <ktoolbarpopupaction.h>
#include <khbox.h>
#include <kvbox.h>
#include <kicon.h>
#include <kxmlguifactory.h>
#include <kcmultidialog.h>

#include <QComboBox>
#include <QCursor>
#include <QDBusConnection>
#include <QImage>
#include <QLayout>
#include <QList>
#include <QObject>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>

using namespace Kontact;

// This class extends the normal KDBusServiceStarter.
//
// When a service start is requested, it asks all plugins
// to create their dbus interfaces in addition to the normal
// way of starting a service.
class ServiceStarter : public KDBusServiceStarter
{
  public:

    virtual int startServiceFor( const QString &serviceType,
                                 const QString &constraint = QString(),
                                 QString *error = 0, QString *dbusService = 0,
                                 int flags = 0 );

    // We need to keep track of the plugins which are loaded, so pass a pointer
    // to the plugin list here. Be sure to reset it back to 0 with
    // setPluginList() as soon as the list gets destroyed.
    ServiceStarter( PluginList *pluginList ) {
      mPlugins = pluginList;
    }

    static void setPluginList( PluginList *pluginList ) {
      mPlugins = pluginList;
    }

  protected:

    virtual ~ServiceStarter() {}
    static PluginList *mPlugins;
};

PluginList *ServiceStarter::mPlugins = 0;

int ServiceStarter::startServiceFor( const QString &serviceType,
                                     const QString &constraint,
                                     QString *error, QString *dbusService,
                                     int flags )
{
  if ( mPlugins ) {
    PluginList::ConstIterator end = mPlugins->end();
    for ( PluginList::ConstIterator it = mPlugins->begin(); it != end; ++it ) {
      if ( (*it)->createDBUSInterface( serviceType ) ) {
        kDebug() << "found interface for" << serviceType;
        if ( dbusService ) {
          *dbusService = (*it)->registerClient();
        }
        return 0;
      }
    }
  }

  kDebug() << "Didn't find dbus interface, falling back to external process";
  return KDBusServiceStarter::startServiceFor( serviceType, constraint,
                                               error, dbusService, flags );
}

MainWindow::MainWindow()
  : Kontact::Core(), mSplitter( 0 ), mCurrentPlugin( 0 ), mAboutDialog( 0 ),
    mReallyClose( false ), mSyncActionsEnabled( true )
{
  // The ServiceStarter created here will be deleted by the KDbusServiceStarter
  // base class, which is a global static.
  new ServiceStarter( &mPlugins );

  QDBusConnection::sessionBus().registerObject(
    "/KontactInterface", this, QDBusConnection::ExportScriptableSlots );

  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

  initGUI();
  initObject();

  mSidePane->setMaximumWidth( mSidePane->sizeHint().width() );
  mSidePane->setMinimumWidth( mSidePane->sizeHint().width() );

  factory()->plugActionList( this, QString( "navigator_actionlist" ), mActionPlugins );

  restoreWindowSize( KConfigGroup( KGlobal::config(), "MainWindow" ) );
  setAutoSaveSettings();
}

void MainWindow::initGUI()
{
  initWidgets();
  setupActions();
  setHelpMenuEnabled( false );
  KHelpMenu *helpMenu = new KHelpMenu( this, 0, true, actionCollection() );
  connect( helpMenu, SIGNAL(showAboutApplication()), SLOT(showAboutDialog()) );

  KStandardAction::keyBindings( this, SLOT(configureShortcuts()), actionCollection() );
  KStandardAction::configureToolbars( this, SLOT(configureToolbars()), actionCollection() );
  setXMLFile( "kontactui.rc" );

  setStandardToolBarMenuEnabled( true );

  createGUI( 0 );

  connect( Kontact::ProfileManager::self(), SIGNAL( profileLoaded( const QString& ) ),
           this, SLOT( slotLoadProfile( const QString& ) ) );
  connect( Kontact::ProfileManager::self(), SIGNAL( saveToProfileRequested( const QString& ) ),
           this, SLOT( slotSaveToProfile( const QString& ) ) );
}

void MainWindow::initObject()
{
  KService::List offers = KServiceTypeTrader::self()->query(
    QString::fromLatin1( "Kontact/Plugin" ),
    QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  mPluginInfos = KPluginInfo::fromServices(
    offers, KConfigGroup( Prefs::self()->config(), "Plugins" ) );

  KPluginInfo::List::Iterator it;
  for ( it = mPluginInfos.begin(); it != mPluginInfos.end(); ++it ) {
    it->load();
  }

  // prepare the part manager
  mPartManager = new KParts::PartManager( this );
  connect( mPartManager, SIGNAL(activePartChanged(KParts::Part *)),
           this, SLOT(slotActivePartChanged(KParts::Part *)) );

  loadPlugins();

  if ( mSidePane ) {
    mSidePane->updatePlugins();
  }

  KSettings::Dispatcher::registerComponent( componentData(), this, "updateConfig" );

  loadSettings();

  statusBar()->show();

  QTimer::singleShot( 200, this, SLOT( slotShowTipOnStart() ) );

  // done initializing
  slotShowStatusMsg( QString::null );	//krazy:exclude=nullstrassign for old broken gcc

  connect( KPIM::BroadcastStatus::instance(), SIGNAL(statusMsg(const QString &)),
           this, SLOT(slotShowStatusMsg(const QString &)) );

  // launch commandline specified module if any
  activatePluginModule();

  if ( Prefs::lastVersionSeen() == KGlobal::mainComponent().aboutData()->version() ) {
    selectPlugin( mCurrentPlugin );
  }

  paintAboutScreen( introductionString() );
  Prefs::setLastVersionSeen( KGlobal::mainComponent().aboutData()->version() );
}

MainWindow::~MainWindow()
{
  if ( mCurrentPlugin ) {
    saveMainWindowSettings( KGlobal::config()->group(
                              QString( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
  }

  createGUI( 0 );
  ServiceStarter::setPluginList( 0 );
  saveSettings();

  QList<KParts::Part*> parts = mPartManager->parts();

//  Q_FOREACH( KParts::Part *p, parts ) {
//    delete p;
//    p = 0;
//  }

  Prefs::self()->writeConfig();

  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    delete *it;
  }
  // Make sure we really return from the event loop. It could happen that a KJob
  // somewhere is still running because a part forgot to delete it. And a running
  // KJob still has a reference to a KGlobal.
  // Normally KGlobal::deref() calls this when the last reference goes away.
  QCoreApplication::instance()->quit();
}

void MainWindow::setActivePluginModule( const QString &module )
{
  mActiveModule = module;
  activatePluginModule();
}

bool MainWindow::pluginActionWeightLessThan( const QAction *left, const QAction *right )
{
  return pluginWeightLessThan( left->data().value<Kontact::Plugin*>(),
                               right->data().value<Kontact::Plugin*>() );
}

bool MainWindow::pluginWeightLessThan( const Kontact::Plugin *left, const Kontact::Plugin *right )
{
  return left->weight() < right->weight();
}

void MainWindow::activatePluginModule()
{
  if ( !mActiveModule.isEmpty() ) {
    PluginList::ConstIterator end = mPlugins.end();
    for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
      if ( ( *it )->identifier().contains( mActiveModule ) ) {
        selectPlugin( *it );
        return;
      }
    }
  }
}

void MainWindow::initWidgets()
{
  QWidget *mTopWidget = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  mTopWidget->setLayout( layout );
  setCentralWidget( mTopWidget );

  mSplitter = new QSplitter( mTopWidget );
  layout->addWidget( mSplitter );
  mSidePane = new IconSidePane( this, mSplitter );
/*
  // don't occupy screen estate on load

  QList<int> sizes;
  sizes << 0;
  mSplitter->setSizes(sizes);
*/
  connect( mSidePane, SIGNAL(pluginSelected(Kontact::Plugin *)),
           SLOT(selectPlugin(Kontact::Plugin *)) );

  mPartsStack = new QStackedWidget( mSplitter );
  mPartsStack->layout()->setSpacing( 0 );

  initAboutScreen();

  QString loading =
    i18n( "<h2 style='text-align:center; margin-top: 0px; margin-bottom: 0px'>%1</h2>",
          i18n( "Loading Kontact..." ) );

  paintAboutScreen( loading );

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

  mSplitter->setCollapsible( 1, false );
}

void MainWindow::paintAboutScreen( const QString &msg )
{
  QString location = KStandardDirs::locate( "data", "kontact/about/main.html" );
  QString content = KPIMUtils::kFileToByteArray( location );
  content = content.arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage.css" ) );
  if ( QApplication::isRightToLeft() ) {
    content = content.arg( "@import \"%1\";" ).
              arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage_rtl.css" ) );
  } else {
    content = content.arg( "" );
  }

  mIntroPart->begin( KUrl( location ) );

  QString appName( i18n( "KDE Kontact" ) );
  QString catchPhrase( i18n( "Get Organized!" ) );
  QString quickDescription( i18n( "The KDE Personal Information Management Suite" ) );

  mIntroPart->write(
    content.arg( QFont().pointSize() + 2 ).
    arg( appName ).arg( catchPhrase ).arg( quickDescription ).arg( msg ) );
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
           SIGNAL(openUrlRequest(const KUrl &,const KParts::OpenUrlArguments &,
                                 const KParts::BrowserArguments &)),
           SLOT(slotOpenUrl(const KUrl &)) );

  connect( mIntroPart->browserExtension(),
           SIGNAL(createNewWindow(const KUrl &,const KParts::OpenUrlArguments &,
                                  const KParts::BrowserArguments &)),
           SLOT(slotOpenUrl(const KUrl&)) );
}

void MainWindow::setupActions()
{
  actionCollection()->addAction( KStandardAction::Quit, this, SLOT(slotQuit()) );

#ifdef Q_WS_MACX
 // ### This quits the application prematurely, for example when the composer window
 // ### is closed while the main application is minimized to the systray
  connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
#endif

  mNewActions = new KToolBarPopupAction(
    KIcon( "" ),
    i18nc( "@title:menu create new pim items (message,calendar,to-do,etc.)", "New" ), this );
  actionCollection()->addAction( "action_new", mNewActions );
  mNewActions->setShortcut( KStandardShortcut::openNew() );
  connect( mNewActions, SIGNAL(triggered(bool)), this, SLOT(slotNewClicked()) );

  // If the user is using disconnected imap mail folders as groupware, we add plugins' Synchronize
  // actions to the toolbar which trigger an imap sync.  Otherwise it's redundant and
  // misleading.
  KConfig *_cfg = Prefs::self()->config();
  KConfigGroup cfg( _cfg, "Kontact Groupware Settings" );
  mSyncActionsEnabled = cfg.readEntry( "GroupwareMailFoldersEnabled", true );

  if ( mSyncActionsEnabled ) {
    mSyncActions = new KToolBarPopupAction(
      KIcon( "view-refresh" ),
      i18nc( "@title:menu synchronize pim items (message,calendar,to-do,etc.)", "Sync" ), this );
    actionCollection()->addAction( "action_sync", mSyncActions );
    mSyncActions->setShortcut( KStandardShortcut::reload() );
    connect( mSyncActions, SIGNAL(triggered(bool)), this, SLOT(slotSyncClicked()) );
  }

  KAction *action = new KAction( KIcon( "configure" ), i18n( "Configure Kontact..." ), this );
  actionCollection()->addAction( "settings_configure_kontact", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotPreferences()) );

  action = new KAction( i18n( "Configure &Profiles..." ), this );
  actionCollection()->addAction( "settings_configure_kontact_profiles", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotConfigureProfiles()) );

  action = new KAction( KIcon( "kontact" ), i18n( "&Kontact Introduction" ), this );
  actionCollection()->addAction( "help_introduction", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotShowIntroduction()) );
  action = new KAction( KIcon( "ktip" ), i18n( "&Tip of the Day" ), this );
  actionCollection()->addAction( "help_tipofday", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotShowTip()) );
}

void MainWindow::slotConfigureProfiles()
{
  Kontact::ProfileDialog *dlg = new Kontact::ProfileDialog( this );
  dlg->setModal( true );
  dlg->exec();
  delete dlg;
}

namespace {
    void copyConfigEntry( KConfig* source, KConfig* dest, const QString& group, const QString& key, const QString& defaultValue=QString() )
    {
        KConfigGroup src = source->group( group );
        KConfigGroup dst = dest->group( group );
        dst.writeEntry( key, src.readEntry( key, defaultValue ) );
    }

    void copyConfigEntry( KConfig* source, KConfig* dest, const QString& group, const QString& subgroup, const QString& key, const QString& defaultValue=QString() )
    {
        KConfigGroup src = source->group( group );
        KConfigGroup dst = dest->group( group );
        KConfigGroup subsrc = src.group( subgroup );
        KConfigGroup subdst = dst.group( subgroup );
        subdst.writeEntry( key, subsrc.readEntry( key, defaultValue ) );
    }
}

void MainWindow::slotSaveToProfile( const QString& id )
{
  const QString path = Kontact::ProfileManager::self()->profileById( id ).saveLocation();
  if ( path.isNull() )
    return;

  KConfig* const cfg = Prefs::self()->config();
  Prefs::self()->writeConfig();
  saveMainWindowSettings( cfg->group("MainWindow") );
  saveSettings();

  KConfig profile( path+"/kontactrc", KConfig::NoGlobals );
  ::copyConfigEntry( cfg, &profile, "MainWindow", "Toolbar navigatorToolBar", "Hidden", "true" );
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
  saveMainWindowSettings( cfg->group("MainWindow") );
  saveSettings();

  const KConfig profile( path+"/kontactrc", KConfig::NoGlobals );
  const QStringList groups = profile.groupList();
  for ( QStringList::ConstIterator it = groups.begin(), end = groups.end(); it != end; ++it )
  {
    KConfigGroup group = cfg->group( *it );
    typedef QMap<QString, QString> StringMap;
    const StringMap entries = profile.entryMap( *it );
    for ( StringMap::ConstIterator it2 = entries.begin(), end = entries.end(); it2 != end; ++it2 )
    {
      if ( it2.value() == "KONTACT_PROFILE_DELETE_KEY" )
        group.deleteEntry( it2.key() );
      else
        group.writeEntry( it2.key(), it2.value() );
    }
  }

  cfg->sync();
  Prefs::self()->readConfig();
  applyMainWindowSettings( cfg->group("MainWindow") );
  KIconTheme::reconfigure();
  KGlobalSettings::emitChange( KGlobalSettings::PaletteChanged );
  KGlobalSettings::emitChange( KGlobalSettings::FontChanged );
  KGlobalSettings::emitChange( KGlobalSettings::StyleChanged );
  KGlobalSettings::emitChange( KGlobalSettings::SettingsChanged );
  for ( int i = 0; i < KIconLoader::LastGroup; ++i )
      KGlobalSettings::emitChange( KGlobalSettings::IconChanged, i );

  loadSettings();

  for ( PluginList::Iterator it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
    if ( !(*it)->isRunningStandalone() ) {
        kDebug() << "Ensure loaded: " << (*it)->identifier();
        (*it)->part();
    }
    (*it)->loadProfile( path );
  }
}

bool MainWindow::isPluginLoaded( const KPluginInfo &info )
{
  return ( pluginFromInfo( info ) != 0 );
}

Plugin *MainWindow::pluginFromInfo( const KPluginInfo &info )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    if ( (*it)->identifier() == info.pluginName() ) {
      return *it;
    }
  }
  return 0;
}

void MainWindow::loadPlugins()
{
  QList<Plugin*> plugins;

  int i;
  KPluginInfo::List::ConstIterator it;
  for ( it = mPluginInfos.begin(); it != mPluginInfos.end(); ++it ) {
    if ( !it->isPluginEnabled() ) {
      continue;
    }

    Plugin *plugin = 0;
    if ( isPluginLoaded( *it ) ) {
      plugin = pluginFromInfo( *it );
      if ( plugin ) {
        plugin->configUpdated();
      }
      continue;
    }

    kDebug() << "Loading Plugin:" << it->name();
    plugin =  KService::createInstance<Kontact::Plugin>( it->service(), this );

    if ( !plugin ) {
      kDebug() << "Unable to create plugin for " << it->name();
      continue;
    }

    plugin->setIdentifier( it->pluginName() );
    plugin->setTitle( it->name() );
    plugin->setIcon( it->icon() );

    QVariant libNameProp = it->property( "X-KDE-KontactPartLibraryName" );
    QVariant exeNameProp = it->property( "X-KDE-KontactPartExecutableName" );
    QVariant loadOnStart = it->property( "X-KDE-KontactPartLoadOnStart" );
    QVariant hasPartProp = it->property( "X-KDE-KontactPluginHasPart" );

    if ( !loadOnStart.isNull() && loadOnStart.toBool() ) {
      mDelayedPreload.append( plugin );
    }

    kDebug() << "LIBNAMEPART:" << libNameProp.toString();

    plugin->setPartLibraryName( libNameProp.toString().toUtf8() );
    plugin->setExecutableName( exeNameProp.toString() );
    if ( hasPartProp.isValid() ) {
      plugin->setShowInSideBar( hasPartProp.toBool() );
    }

    for ( i = 0; i < plugins.count(); ++i ) {
      Plugin *p = plugins.at( i );
      if ( plugin->weight() < p->weight() ) {
        break;
      }
    }
    plugins.insert( i, plugin );

  }

  for ( i = 0; i < plugins.count(); ++ i ) {
    Plugin *plugin = plugins.at( i );

    const QList<KAction*> *actionList = plugin->newActions();
    QList<KAction*>::const_iterator listIt;

    for ( listIt = actionList->begin(); listIt != actionList->end(); ++listIt ) {
      kDebug() << "Plugging New actions" << (*listIt)->objectName();
      mNewActions->menu()->addAction( (*listIt) );
    }

    actionList = plugin->syncActions();
    if ( mSyncActionsEnabled ) {
      Q_FOREACH( KAction *listIt, *actionList ) {
        kDebug() << "Plugging Sync actions" << listIt->objectName();
        mSyncActions->menu()->addAction( listIt );
      }
    }
    addPlugin( plugin );
  }

  mNewActions->setEnabled( mPlugins.size() != 0 );
  if ( mSyncActionsEnabled ) {
    mSyncActions->setEnabled( mPlugins.size() != 0 );
  }
}

void MainWindow::unloadPlugins()
{
  KPluginInfo::List::ConstIterator end = mPluginInfos.end();
  KPluginInfo::List::ConstIterator it;
  for ( it = mPluginInfos.begin(); it != end; ++it ) {
    if ( !it->isPluginEnabled() ) {
      removePlugin( *it );
    }
  }
}

void MainWindow::updateShortcuts()
{
  ActionPluginList::ConstIterator end = mActionPlugins.end();
  ActionPluginList::ConstIterator it;
  int i = 0;
  for ( it = mActionPlugins.begin(); it != end; ++it ) {
    KAction *action = static_cast<KAction*>( *it );
    QString shortcut = QString( "Ctrl+%1" ).arg( mActionPlugins.count() - i );
    action->setShortcut( KShortcut( shortcut ) );
    i++;
  }
  factory()->plugActionList( this, QString( "navigator_actionlist" ), mActionPlugins );
}

bool MainWindow::removePlugin( const KPluginInfo &info )
{
  PluginList::Iterator end = mPlugins.end();
  ActionPluginList::Iterator it2 = mActionPlugins.begin();
  for ( PluginList::Iterator it = mPlugins.begin(); it != end; ++it ) {
    Plugin *plugin = *it;
    if ( ( *it )->identifier() == info.pluginName() ) {
      const QList<KAction*> *actionList = plugin->newActions();
      QList<KAction*>::const_iterator listIt;

      for ( listIt = actionList->begin(); listIt != actionList->end(); ++listIt ) {
        kDebug() << "Unplugging New actions" << (*listIt)->objectName();
        mNewActions->menu()->removeAction( *listIt );
      }

      if ( mSyncActionsEnabled ) {
        actionList = plugin->syncActions();
        for ( listIt = actionList->begin(); listIt != actionList->end(); ++listIt ) {
            kDebug() << "Unplugging Sync actions" << (*listIt)->objectName();
            mSyncActions->menu()->removeAction( *listIt );
        }
      }
      removeChildClient( plugin );

      if ( mCurrentPlugin == plugin ) {
        mCurrentPlugin = 0;
        createGUI( 0 );
      }

      plugin->deleteLater(); // removes the part automatically
      mPlugins.erase( it );
      if ( plugin->showInSideBar() ) {
        delete *it2; // remove the KAction, so we free the shortcut for later us
        mActionPlugins.erase( it2 );
      }

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

    if ( plugin->showInSideBar() ) {
      it2++;
    }
  }

  return false;
}

void MainWindow::addPlugin( Kontact::Plugin *plugin )
{
  kDebug();

  mPlugins.append( plugin );

  if ( plugin->showInSideBar() ) {
    KAction *action = new KAction( KIcon( plugin->icon() ), plugin->title(), this );
    action->setData( QVariant::fromValue( plugin ) ); // on the slot we can decode which action was
                                                      // triggered
    connect( action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()) );
    actionCollection()->addAction( plugin->title(), action );
    mActionPlugins.append( action );
  }

  // merge the plugins GUI into the main window
  insertChildClient( plugin );

  // sort the action plugins again and reset shortcuts. If we removed and then readded some plugins
  // we need to take in count their weights for setting shortcuts again
  qSort( mActionPlugins.begin(), mActionPlugins.end(), pluginActionWeightLessThan );
  qSort( mPlugins.begin(), mPlugins.end(), pluginWeightLessThan );
  int i = 0;
  foreach ( QAction *qaction, mActionPlugins ) {
    KAction *action = static_cast<KAction*>( qaction );
    QString shortcut = QString( "Ctrl+%1" ).arg( mActionPlugins.count() - i );
    action->setShortcut( KShortcut( shortcut ) );
    i++;
  }
}

void MainWindow::partLoaded( Kontact::Plugin *plugin, KParts::ReadOnlyPart *part )
{
  Q_UNUSED( plugin );

  // See if we have this part already (e.g. due to two plugins sharing it)
  if ( mPartsStack->indexOf( part->widget() ) != -1 ) {
    return;
  }

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

  kDebug() << "Part activated:" << part
           << "with stack id."<< mPartsStack->indexOf( part->widget() );

  statusBar()->clearMessage();
}

void MainWindow::slotNewClicked()
{
  if ( !mCurrentPlugin->newActions()->isEmpty() ) {
    mCurrentPlugin->newActions()->first()->trigger();
  } else {
    PluginList::Iterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      if ( !(*it)->newActions()->isEmpty() ) {
        (*it)->newActions()->first()->trigger();
        return;
      }
    }
  }
}

void MainWindow::slotSyncClicked()
{
  if ( !mCurrentPlugin->syncActions()->isEmpty() ) {
    mCurrentPlugin->syncActions()->first()->trigger();
  } else {
    PluginList::Iterator it;
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      if ( !(*it)->syncActions()->isEmpty() ) {
        (*it)->syncActions()->first()->trigger();
        return;
      }
    }
  }
}

KToolBar *Kontact::MainWindow::findToolBar( const char *name )
{
  // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
  return findChild<KToolBar *>( name );
}

void MainWindow::selectPlugin( Kontact::Plugin *plugin )
{
  if ( !plugin ) {
    return;
  }

  if ( plugin->isRunningStandalone() ) {
    statusBar()->showMessage( i18n( "Application is running standalone. Foregrounding..." ), 1000 );
    plugin->bringToForeground();
    return;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if ( mCurrentPlugin ) {
    saveMainWindowSettings( KGlobal::config()->group(
                              QString( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
  }

  KParts::Part *part = plugin->part();

  if ( !part ) {
    QApplication::restoreOverrideCursor();
    KMessageBox::error( this, i18n( "Cannot load part for %1.",
                                    plugin->title() ) + '\n' + lastErrorMessage() );
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
      if ( parent == mCurrentPlugin->part()->widget() ) {
        mFocusWidgets.insert( mCurrentPlugin->identifier(), QPointer<QWidget>( focusWidget ) );
      }
      parent = parent->parentWidget();
    }
  }

  if ( mSidePane ) {
    mSidePane->setCurrentPlugin( plugin->identifier() );
  }

  plugin->select();

  mPartManager->setActivePart( part );
  QWidget *view = part->widget();
  Q_ASSERT( view );

  if ( view ) {
    mPartsStack->setCurrentWidget( view );
    view->show();

    if ( mFocusWidgets.contains( plugin->identifier() ) ) {
      focusWidget = mFocusWidgets[ plugin->identifier() ];
      if ( focusWidget ) {
        focusWidget->setFocus();
      }
    } else {
      view->setFocus();
    }

    mCurrentPlugin = plugin;

    QAction *newAction = 0;
    if ( plugin->newActions()->count() > 0 ) {
      newAction = plugin->newActions()->first();
    }

    QAction *syncAction = 0;
    if ( plugin->syncActions()->count() > 0 ) {
      syncAction = plugin->syncActions()->first();
    }

    createGUI( plugin->part() );

    KToolBar *navigatorToolBar = findToolBar( "navigatorToolBar" );
    // Let the navigator toolbar be always the last one, if it's in the top dockwindow
    if ( navigatorToolBar && !navigatorToolBar->isHidden() &&
         toolBarArea( navigatorToolBar ) == Qt::TopToolBarArea ) {
      addToolBar( Qt::TopToolBarArea, navigatorToolBar );
    }

    setCaption( i18nc( "Plugin dependent window title", "%1 - Kontact", plugin->title() ) );

    if ( newAction ) {
      mNewActions->setIcon( newAction->icon() );
      static_cast<QAction*>( mNewActions )->setText( newAction->text() );
    } else { // we'll use the action of the first plugin which offers one
      PluginList::Iterator it;
      for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
        if ( (*it)->newActions()->count() > 0 ) {
          newAction = (*it)->newActions()->first();
        }
        if ( newAction ) {
          static_cast<QAction*>( mNewActions )->setIcon( newAction->icon() );
          mNewActions->setText( newAction->text() );
          break;
        }
      }
    }

    if ( mSyncActionsEnabled ) {
      if ( syncAction ) {
        mSyncActions->setIcon( syncAction->icon() );
        static_cast<QAction*>( mSyncActions )->setText( syncAction->text() );
      } else { // we'll use the action of the first plugin which offers one
        PluginList::Iterator it;
        for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
          if ( (*it)->syncActions()->count() > 0 ) {
            syncAction = (*it)->syncActions()->first();
          }
          if ( syncAction ) {
            static_cast<QAction*>( mSyncActions )->setIcon( syncAction->icon() );
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
    QAction *action = part->actionCollection()->action( (*it) );
    if ( action ) {
      QList<KToolBar*> toolbars = toolBars();
      for ( QList<KToolBar*>::Iterator it = toolbars.begin(); it != toolbars.end(); ++it ) {
        ( *it )->removeAction( action );
      }
    }
  }

  applyMainWindowSettings( KGlobal::config()->group(
                             QString( "MainWindow%1" ).arg( plugin->identifier() ) ) );

  QApplication::restoreOverrideCursor();
}

void MainWindow::slotActionTriggered()
{
  KAction *actionSender = static_cast<KAction*>( sender() );
  Kontact::Plugin *plugin = actionSender->data().value<Kontact::Plugin*>();
  if ( !plugin ) {
    return;
  }
  mSidePane->setCurrentPlugin( plugin->identifier() );
}

void MainWindow::selectPlugin( const QString &pluginName )
{
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    if ( ( *it )->identifier() == pluginName ) {
      selectPlugin( *it );
      return;
    }
  }
}

void MainWindow::loadSettings()
{
  if ( mSplitter ) {
    // if the preferences do not contain useful values, the side pane part of the splitter
    // takes up the full width of the window, so leave the splitter sizing at the widget defaults
    QList<int> sizes = Prefs::self()->sidePaneSplitter();
    if ( sizes.count() == mSplitter->count() ) {
      mSplitter->setSizes( sizes );
    }
  }

  // Preload Plugins. This _must_ happen before the default part is loaded
  PluginList::ConstIterator it;
  for ( it = mDelayedPreload.begin(); it != mDelayedPreload.end(); ++it ) {
    selectPlugin( *it );
  }
  selectPlugin( Prefs::self()->mActivePlugin );
}

void MainWindow::saveSettings()
{
  if ( mSplitter ) {
    Prefs::self()->mSidePaneSplitter = mSplitter->sizes();
  }

  if ( mCurrentPlugin ) {
    Prefs::self()->mActivePlugin = mCurrentPlugin->identifier();
  }
}

void MainWindow::slotShowTip()
{
  showTip( true );
}

void MainWindow::slotShowTipOnStart()
{
  showTip( false );
}

void MainWindow::slotShowIntroduction()
{
  mPartsStack->setCurrentIndex( 0 );
}

void MainWindow::showTip( bool force )
{
  QStringList tips;
  PluginList::ConstIterator end = mPlugins.end();
  for ( PluginList::ConstIterator it = mPlugins.begin(); it != end; ++it ) {
    QString file = (*it)->tipFile();
    if ( !file.isEmpty() ) {
      tips.append( file );
    }
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
    for ( it = mPlugins.begin(); it != mPlugins.end(); ++it ) {
      if ( (*it)->isRunningStandalone() ) {
        KPluginInfo::List::ConstIterator infoIt;
        for ( infoIt = filteredPlugins.begin(); infoIt != filteredPlugins.end(); ++infoIt ) {
          if ( infoIt->pluginName() == (*it)->identifier() ) {
            filteredPlugins.removeAll( *infoIt );
            break;
          }
        }
      }
    }

    dlg->setHelp( "main-config", "kontact" );
    dlg->addPluginInfos( filteredPlugins );
    connect( dlg, SIGNAL(pluginSelectionChanged()), SLOT(pluginsChanged()) );
  }

  dlg->show();
}

void MainWindow::pluginsChanged()
{
  unplugActionList( "navigator_actionlist" );
  unloadPlugins();
  loadPlugins();
  mSidePane->updatePlugins();
  updateShortcuts();
}

void MainWindow::updateConfig()
{
  kDebug();

  saveSettings();
  loadSettings();
}

void MainWindow::showAboutDialog()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if ( !mAboutDialog ) {
    mAboutDialog = new AboutDialog( this );
  }

  mAboutDialog->show();
  mAboutDialog->raise();
  QApplication::restoreOverrideCursor();
}

void MainWindow::configureShortcuts()
{
  KShortcutsDialog dialog(
    KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this );
  dialog.addCollection( actionCollection() );

  if ( mCurrentPlugin && mCurrentPlugin->part() ) {
    dialog.addCollection( mCurrentPlugin->part()->actionCollection() );
  }

  dialog.configure();
}

void MainWindow::configureToolbars()
{
  if ( mCurrentPlugin ) {
    saveMainWindowSettings( KGlobal::config()->group(
                              QString( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
  }
  KEditToolBar edit( factory() );
  connect( &edit, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()) );
  edit.exec();
}

void MainWindow::slotNewToolbarConfig()
{
  if ( mCurrentPlugin && mCurrentPlugin->part() ) {
    createGUI( mCurrentPlugin->part() );
  }
  if ( mCurrentPlugin ) {
    applyMainWindowSettings( KGlobal::config()->group(
                               QString( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
  }
}

void MainWindow::slotOpenUrl( const KUrl &url )
{
  if ( url.protocol() == "exec" ) {
    if ( url.path() == "/switch" ) {
      if ( mCurrentPlugin ) {
        mPartsStack->setCurrentIndex( mPartsStack->indexOf( mCurrentPlugin->part()->widget() ) );
      }
    }
    if ( url.path() == "/gwwizard" ) {
      KRun::runCommand( "groupwarewizard", this );
      slotQuit();
    }
  } else {
    new KRun( url, this );
  }
}

void MainWindow::readProperties( const KConfigGroup &config )
{
  Core::readProperties( config );

  QSet<QString> activePlugins =
    QSet<QString>::fromList( config.readEntry( "ActivePlugins", QStringList() ) );
  foreach ( Plugin *plugin, mPlugins ) {
    if ( !plugin->isRunningStandalone() && activePlugins.contains( plugin->identifier() ) ) {
      plugin->readProperties( config );
    }
  }
}

void MainWindow::saveProperties( KConfigGroup &config )
{
  Core::saveProperties( config );

  QStringList activePlugins;

  foreach ( const KPluginInfo &pluginInfo, mPluginInfos ) {
    if ( pluginInfo.isPluginEnabled() ) {
      Plugin *plugin = pluginFromInfo( pluginInfo );
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
  if ( kapp->sessionSaving() || mReallyClose ) {
    return true;
  }

  foreach ( Plugin *plugin, mPlugins ) {
    if ( !plugin->isRunningStandalone() ) {
      if ( !plugin->queryClose() ) {
        return false;
      }
    }
  }
  return true;
}

void MainWindow::slotShowStatusMsg( const QString &msg )
{
  if ( !statusBar() || !mStatusMsgLabel ) {
     return;
  }

  mStatusMsgLabel->setText( msg );
}

QString MainWindow::introductionString()
{
  KIconLoader *iconloader = KIconLoader::global();
  int iconSize = iconloader->currentSize( KIconLoader::Desktop );

  QString handbook_icon_path = iconloader->iconPath( "help-contents", KIconLoader::Desktop );
  QString html_icon_path = iconloader->iconPath( "text-html", KIconLoader::Desktop );
  QString wizard_icon_path = iconloader->iconPath( "tools-wizard", KIconLoader::Desktop );

  QString info = ki18n( "<h2 style='text-align:center; margin-top: 0px;'>Welcome to Kontact %1</h2>"
      "<p align=\"center\">%2</p>"
      "<table align=\"center\">"
      "<tr><td><a href=\"%3\"><img width=\"%4\" height=\"%5\" src=\"%6\" /></a></td>"
      "<td><a href=\"%7\">%8</a><br /><span id=\"subtext\"><nobr>%9</nobr></span></td></tr>"
      "<tr><td><a href=\"%10\"><img width=\"%11\" height=\"%12\" src=\"%13\" /></a></td>"
      "<td><a href=\"%14\">%15</a><br /><span id=\"subtext\"><nobr>%16</nobr></span></td></tr>"
      "<tr><td><a href=\"%17\"><img width=\"%18\" height=\"%19\" src=\"%20\" /></a></td>"
      "<td><a href=\"%21\">%22</a><br /><span id=\"subtext\"><nobr>%23</nobr></span></td></tr>"
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
