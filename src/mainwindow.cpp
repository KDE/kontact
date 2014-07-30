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

#include <config-enterprise.h>

#include "mainwindow.h"
#include "aboutdialog.h"
#include "prefs.h"
#include "iconsidepane.h"
#include "kontactconfiguredialog.h"
using namespace Kontact;

#include <unistd.h>

#include <libkdepim/misc/broadcaststatus.h>
#include <libkdepim/progresswidget/progressstatusbarwidget.h>
#include <libkdepim/progresswidget/statusbarprogresswidget.h>

#include <KontactInterface/Core>

#include <KPIMUtils/KFileIO>

#include <QStatusBar>
#include <KXMLGUIFactory>
#include <KActionCollection>
#include <KActionMenu>
#include <KApplication>
#include <KConfigGroup>
#include <KDBusServiceStarter>
#include <KDebug>
#include <KEditToolBar>
#include <KHelpMenu>
#include <KWebView>
#include <KMessageBox>
#include <KPluginInfo>
#include <KRun>
#include <KServiceTypeTrader>
#include <KShortcutsDialog>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KStandardDirs>
#include <KTipDialog>
#include <KToolBar>
#include <KParts/PartManager>
#include <KSettings/Dispatcher>
#include <KSycoca>
#include <KIconLoader>
#include <KGlobal>
#include <KShortcut>
#include <k4aboutdata.h>
#include <KLocalizedString>
#include <QDBusConnection>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebSettings>
#include <QShortcut>
#include <QIcon>
#include <KHelpClient>
#include <KSharedConfig>
#include <QStandardPaths>
#include <QHBoxLayout>

// Define the maximum time Kontact waits for KSycoca to become available.
static const int KSYCOCA_WAIT_TIMEOUT = 10;

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
        PluginList::ConstIterator end = mPlugins->constEnd();
        for ( PluginList::ConstIterator it = mPlugins->constBegin(); it != end; ++it ) {
            if ( (*it)->createDBUSInterface( serviceType ) ) {
                qDebug() << "found interface for" << serviceType;
                if ( dbusService ) {
                    *dbusService = (*it)->registerClient();
                }
                return 0;
            }
        }
    }

    qDebug() << "Didn't find dbus interface, falling back to external process";
    return KDBusServiceStarter::startServiceFor( serviceType, constraint,
                                                 error, dbusService, flags );
}

MainWindow::MainWindow()
    : KontactInterface::Core(), mSplitter( 0 ), mCurrentPlugin( 0 ), mAboutDialog( 0 ),
      mReallyClose( false ), mSyncActionsEnabled( true )
{
    // The ServiceStarter created here will be deleted by the KDbusServiceStarter
    // base class, which is a global static.
    new ServiceStarter( &mPlugins );

    QDBusConnection::sessionBus().registerObject(
                QLatin1String("/KontactInterface"), this, QDBusConnection::ExportScriptableSlots );

    // Set this to be the group leader for all subdialogs - this means
    // modal subdialogs will only affect this dialog, not the other windows
    setAttribute( Qt::WA_GroupLeader );

    initGUI();
    initObject();

    mSidePane->setMaximumWidth( mSidePane->sizeHint().width() );
    mSidePane->setMinimumWidth( mSidePane->sizeHint().width() );

    factory()->plugActionList( this, QLatin1String( "navigator_actionlist" ), mActionPlugins );

    restoreWindowSize( KConfigGroup( KSharedConfig::openConfig(), "MainWindow" ) );
    setAutoSaveSettings();
}

void MainWindow::initGUI()
{
    initWidgets();
    setupActions();
    setHelpMenuEnabled( false );
    KHelpMenu *helpMenu = new KHelpMenu( this, QString(), true/*, actionCollection() QT5*/ );
    connect( helpMenu, SIGNAL(showAboutApplication()), SLOT(showAboutDialog()) );

    KStandardAction::keyBindings( this, SLOT(configureShortcuts()), actionCollection() );
    KStandardAction::configureToolbars( this, SLOT(configureToolbars()), actionCollection() );
    setXMLFile( QLatin1String("kontactui.rc") );

    setStandardToolBarMenuEnabled( true );

    createGUI( 0 );

    KToolBar *navigatorToolBar = findToolBar( "navigatorToolBar" );
    if ( navigatorToolBar ) {
        if ( layoutDirection() == Qt::LeftToRight ) {
            navigatorToolBar->setLayoutDirection( Qt::RightToLeft );
        } else {
            navigatorToolBar->setLayoutDirection( Qt::LeftToRight );
        }
        Q_ASSERT( navigatorToolBar->sizeHint().isValid() );
        navigatorToolBar->setMinimumWidth( navigatorToolBar->sizeHint().width() );
    } else {
        qCritical() << "Unable to find navigatorToolBar, probably kontactui.rc is missing";
    }
}

void MainWindow::waitForKSycoca()
{
    int i = 0;
    while ( i < KSYCOCA_WAIT_TIMEOUT ) {
        if ( KSycoca::isAvailable() ) {
            return;
        }
        // When KSycoca is not availabe that usually means Kontact
        // was started before kded is done with it's first run
        // we want to block Kontact execution to
        // give Kded time to initialize and create the
        // System Configuration database necessary for further
        // Kontact startup
        qDebug() << "Waiting for KSycoca";
        sleep(1);
        ++i;
    }
    // This should only happen if the distribution is broken
    qFatal("KSycoca unavailable. Kontact will be unable to find plugins.");
}

void MainWindow::initObject()
{
    if ( !KSycoca::isAvailable() ) {
        waitForKSycoca();
    }
    KService::List offers = KServiceTypeTrader::self()->query(
                QString::fromLatin1( "Kontact/Plugin" ),
                QString::fromLatin1( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
#if 0 //QT5
    mPluginInfos = KPluginInfo::fromServices(
                offers, KConfigGroup( Prefs::self()->config(), "Plugins" ) );
#endif
    KPluginInfo::List::Iterator it;
    KPluginInfo::List::Iterator end( mPluginInfos.end() );

    for ( it = mPluginInfos.begin(); it != end; ++it ) {
        it->load();
    }

    // prepare the part manager
    mPartManager = new KParts::PartManager( this );
    connect( mPartManager, SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    loadPlugins();

    if ( mSidePane ) {
        mSidePane->updatePlugins();
    }

    //QT5 KSettings::Dispatcher::registerComponent( componentData(), this, "updateConfig" );

    loadSettings();

    statusBar()->show();

    QTimer::singleShot( 200, this, SLOT(slotShowTipOnStart()) );

    // done initializing
    slotShowStatusMsg( QString::null );        //krazy:exclude=nullstrassign for old broken gcc

    connect( KPIM::BroadcastStatus::instance(), SIGNAL(statusMsg(QString)),
             this, SLOT(slotShowStatusMsg(QString)) );

    // launch commandline specified module if any
    activateInitialPluginModule();

    if ( Prefs::lastVersionSeen() == KComponentData::mainComponent().aboutData()->version() ) {
        selectPlugin( mCurrentPlugin );
    }

    paintAboutScreen( introductionString() );
    Prefs::setLastVersionSeen( KComponentData::mainComponent().aboutData()->version() );
}

MainWindow::~MainWindow()
{
    if ( mCurrentPlugin ) {
        KConfigGroup grp(
                    KSharedConfig::openConfig()->group(
                        QString::fromLatin1( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
        saveMainWindowSettings( grp );
    }

    createGUI( 0 );
    ServiceStarter::setPluginList( 0 );
    saveSettings();

    //QList<KParts::Part*> parts = mPartManager->parts();

    //  Q_FOREACH( KParts::Part *p, parts ) {
    //    delete p;
    //    p = 0;
    //  }

    Prefs::self()->save();

    // During deletion of plugins, we should not access the plugin list (bug #182176)
    delete mSidePane;

    PluginList::ConstIterator end = mPlugins.constEnd();
    for ( PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it ) {
        delete *it;
    }
}

// Called by main().
void MainWindow::setInitialActivePluginModule( const QString &module )
{
    if (mInitialActiveModule != module) {
        mInitialActiveModule = module;
        activateInitialPluginModule();
    }
}

bool MainWindow::pluginActionWeightLessThan( const QAction *left, const QAction *right )
{
    // Since this lessThan method is used only for the toolbar (which is on
    // the inverse layout direction than the rest of the system), we add the
    // elements on the exactly inverse order. (ereslibre)
    return !pluginWeightLessThan( left->data().value<KontactInterface::Plugin*>(),
                                  right->data().value<KontactInterface::Plugin*>() );
}

bool MainWindow::pluginWeightLessThan( const KontactInterface::Plugin *left,
                                       const KontactInterface::Plugin *right )
{
    return left->weight() < right->weight();
}

void MainWindow::activateInitialPluginModule()
{
    if ( !mInitialActiveModule.isEmpty() && !mPlugins.isEmpty() ) {
        PluginList::ConstIterator end = mPlugins.constEnd();
        for ( PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it ) {
            if ( !(*it)->identifier().isEmpty() &&
                 (*it)->identifier().contains( mInitialActiveModule ) ) {
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
    connect( mSidePane, SIGNAL(pluginSelected(KontactInterface::Plugin*)),
             SLOT(selectPlugin(KontactInterface::Plugin*)) );

    mPartsStack = new QStackedWidget( mSplitter );
    mPartsStack->layout()->setSpacing( 0 );

    initAboutScreen();

    const QString loading =
            i18nc( "@item",
                   "<h2 style='text-align:center; margin-top: 0px; margin-bottom: 0px'>%1</h2>",
                   i18nc( "@item:intext", "Loading Kontact..." ) );

    paintAboutScreen( loading );

    KPIM::ProgressStatusBarWidget * progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    mStatusMsgLabel =
            new KSqueezedTextLabel( i18nc( "@info:status", " Initializing..." ), statusBar() );
    mStatusMsgLabel->setTextElideMode( Qt::ElideRight );
    mStatusMsgLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    statusBar()->addWidget( mStatusMsgLabel, 10 );
    statusBar()->addPermanentWidget( progressStatusBarWidget->littleProgress(), 0 );

    mSplitter->setCollapsible( 1, false );
}

void MainWindow::paintAboutScreen( const QString &msg )
{
    QString location = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kontact/about/main.html") );
    QString content = QLatin1String(KPIMUtils::kFileToByteArray( location ));
    content = content.arg( QLatin1String("file:") + KStandardDirs::locate(
                               "data", QLatin1String("kdeui/about/kde_infopage.css") ) );
    if ( QApplication::isRightToLeft() ) {
        content =
                content.arg( QLatin1String("@import \"%1\";") ).
                arg( QLatin1String("file:") + KStandardDirs::locate(
                         "data", QLatin1String("kdeui/about/kde_infopage_rtl.css") ) );
    } else {
        content = content.arg( QString() );
    }

    mIntroPart->setHtml(
                content.arg( QFont().pointSize() + 2 ).
                arg( i18nc( "@item:intext", "KDE Kontact" ) ).
                arg( i18nc( "@item:intext", "Get Organized!" ) ).
                arg( i18nc( "@item:intext", "The KDE Personal Information Management Suite" ) ).
                arg( msg ) );
}

void MainWindow::initAboutScreen()
{
    QWidget *introbox = new QWidget( mPartsStack );
    QHBoxLayout *introboxHBoxLayout = new QHBoxLayout(introbox);
    introboxHBoxLayout->setMargin(0);
    mPartsStack->addWidget( introbox );
    mPartsStack->setCurrentWidget( introbox );
    mIntroPart = new KWebView( introbox );
    introboxHBoxLayout->addWidget(mIntroPart);
    mIntroPart->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
    mIntroPart->setFocusPolicy( Qt::WheelFocus );
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    mIntroPart->settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
    mIntroPart->settings()->setAttribute( QWebSettings::JavaEnabled, false );
    mIntroPart->settings()->setAttribute( QWebSettings::PluginsEnabled, false );

    connect( mIntroPart->page(), SIGNAL(linkClicked(QUrl)), this,
             SLOT(slotOpenUrl(QUrl)), Qt::QueuedConnection );
}

void MainWindow::setupActions()
{
    actionCollection()->addAction( KStandardAction::Quit, this, SLOT(slotQuit()) );

    mNewActions = new KActionMenu(
                i18nc( "@title:menu create new pim items (message,calendar,to-do,etc.)", "New" ), this );
    actionCollection()->addAction( QLatin1String("action_new"), mNewActions );
    mNewActions->setShortcuts( KStandardShortcut::openNew() );
    connect( mNewActions, SIGNAL(triggered(bool)), this, SLOT(slotNewClicked()) );

    // If the user is using disconnected imap mail folders as groupware, we add
    // plugins' Synchronize actions to the toolbar which trigger an imap sync.
    // Otherwise it's redundant and misleading.
    KConfig _config( QLatin1String("kmail2rc") );
    KConfigGroup config( &_config, "Groupware" );
#if defined(KDEPIM_ENTERPRISE_BUILD)
    bool defGW = config.readEntry( "Enabled", true );
#else
    bool defGW = config.readEntry( "Enabled", false );
#endif
    KConfig *_cfg = Prefs::self()->config();
    KConfigGroup cfg( _cfg, "Kontact Groupware Settings" );
    mSyncActionsEnabled = cfg.readEntry( "GroupwareMailFoldersEnabled", defGW );

    if ( mSyncActionsEnabled ) {
        mSyncActions = new KActionMenu(
                    QIcon::fromTheme( QLatin1String("view-refresh") ),
                    i18nc( "@title:menu synchronize pim items (message,calendar,to-do,etc.)", "Sync" ), this );
        actionCollection()->addAction( QLatin1String("action_sync"), mSyncActions );
        mSyncActions->setShortcuts( KStandardShortcut::reload() );
        connect( mSyncActions, SIGNAL(triggered(bool)), this, SLOT(slotSyncClicked()) );
    }

    QAction *action =
            new QAction( QIcon::fromTheme( QLatin1String("configure") ),
                         i18nc( "@action:inmenu", "Configure Kontact..." ), this );
    //QT5action->setHelpText(
    //            i18nc( "@info:status", "Configure Kontact" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can configure Kontact." ) );
    actionCollection()->addAction( QLatin1String("settings_configure_kontact"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotPreferences()) );

    action =
            new QAction( QIcon::fromTheme( QLatin1String("kontact") ),
                         i18nc( "@action:inmenu", "&Kontact Introduction" ), this );
    //action->setHelpText(
    //            i18nc( "@info:status", "Show the Kontact Introduction page" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Choose this option to see the Kontact Introduction page." ) );
    actionCollection()->addAction( QLatin1String("help_introduction"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotShowIntroduction()) );

    action =
            new QAction( QIcon::fromTheme( QLatin1String("ktip") ),
                         i18nc( "@action:inmenu", "&Tip of the Day" ), this );
    //action->setHelpText(
    //            i18nc( "@info:status", "Show the Tip-of-the-Day dialog" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog showing small tips to help "
                       "you use this program more effectively." ) );
    actionCollection()->addAction( QLatin1String("help_tipofday"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotShowTip()) );
    //TODO 4.12: add description
    QShortcut *shortcut = new QShortcut( QKeySequence(Qt::Key_F9), this );
    connect(shortcut, SIGNAL(activated()), this, SLOT(slotShowHideSideBar()));
}

bool MainWindow::isPluginLoaded( const KPluginInfo &info )
{
    return ( pluginFromInfo( info ) != 0 );
}

KontactInterface::Plugin *MainWindow::pluginFromInfo( const KPluginInfo &info )
{
    PluginList::ConstIterator end = mPlugins.constEnd();
    for ( PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it ) {
        if ( (*it)->identifier() == info.pluginName() ) {
            return *it;
        }
    }
    return 0;
}

void MainWindow::loadPlugins()
{
    QList<KontactInterface::Plugin *> plugins;

    int i;
    KPluginInfo::List::ConstIterator it;
    KPluginInfo::List::ConstIterator end( mPluginInfos.constEnd() );
    for ( it = mPluginInfos.constBegin(); it != end; ++it ) {
        if ( !it->isPluginEnabled() ) {
            continue;
        }

        KontactInterface::Plugin *plugin = 0;
        if ( isPluginLoaded( *it ) ) {
            plugin = pluginFromInfo( *it );
            if ( plugin ) {
                plugin->configUpdated();
            }
            continue;
        }

        qDebug() << "Loading Plugin:" << it->name();
        QString error;
        plugin =
                it->service()->createInstance<KontactInterface::Plugin>( this, QVariantList(), &error );

        if ( !plugin ) {
            qDebug() << "Unable to create plugin for" << it->name() << error;
            continue;
        }

        plugin->setIdentifier( it->pluginName() );
        plugin->setTitle( it->name() );
        plugin->setIcon( it->icon() );

        QVariant libNameProp = it->property( QLatin1String("X-KDE-KontactPartLibraryName") );
        QVariant exeNameProp = it->property( QLatin1String("X-KDE-KontactPartExecutableName") );
        QVariant loadOnStart = it->property( QLatin1String("X-KDE-KontactPartLoadOnStart") );
        QVariant hasPartProp = it->property( QLatin1String("X-KDE-KontactPluginHasPart") );

        if ( !loadOnStart.isNull() && loadOnStart.toBool() ) {
            mDelayedPreload.append( plugin );
        }

        qDebug() << "LIBNAMEPART:" << libNameProp.toString();

        plugin->setPartLibraryName( libNameProp.toString().toUtf8() );
        plugin->setExecutableName( exeNameProp.toString() );
        if ( hasPartProp.isValid() ) {
            plugin->setShowInSideBar( hasPartProp.toBool() );
        }

        for ( i = 0; i < plugins.count(); ++i ) {
            KontactInterface::Plugin *p = plugins.at( i );
            if ( plugin->weight() < p->weight() ) {
                break;
            }
        }
        plugins.insert( i, plugin );

    }

    const int numberOfPlugins( plugins.count() );
    for ( i = 0; i < numberOfPlugins; ++i ) {
        KontactInterface::Plugin *plugin = plugins.at( i );

        const QList<QAction*> actionList = plugin->newActions();
        QList<QAction*>::const_iterator listIt;
        QList<QAction*>::const_iterator end( actionList.end() );

        for ( listIt = actionList.begin(); listIt != end; ++listIt ) {
            qDebug() << QLatin1String("Plugging New actions") << (*listIt)->objectName();
            mNewActions->addAction( (*listIt) );
        }

        if ( mSyncActionsEnabled ) {
            Q_FOREACH ( QAction *listIt, plugin->syncActions() ) {
                qDebug() << QLatin1String("Plugging Sync actions") << listIt->objectName();
                mSyncActions->addAction( listIt );
            }
        }
        addPlugin( plugin );
    }

    const bool state = ( !mPlugins.isEmpty() );
    mNewActions->setEnabled( state );
    if ( mSyncActionsEnabled ) {
        mSyncActions->setEnabled( state );
    }
}

void MainWindow::unloadPlugins()
{
    KPluginInfo::List::ConstIterator end = mPluginInfos.constEnd();
    KPluginInfo::List::ConstIterator it;
    for ( it = mPluginInfos.constBegin(); it != end; ++it ) {
        if ( !it->isPluginEnabled() ) {
            removePlugin( *it );
        }
    }
}

void MainWindow::updateShortcuts()
{
    ActionPluginList::ConstIterator end = mActionPlugins.constEnd();
    ActionPluginList::ConstIterator it;
    int i = 0;
    for ( it = mActionPlugins.constBegin(); it != end; ++it ) {
        QAction *action = static_cast<QAction*>( *it );
        const QString shortcut = QString::fromLatin1( "Ctrl+%1" ).arg( mActionPlugins.count() - i );
        action->setShortcut( QKeySequence( shortcut ) );
        ++i;
    }
    factory()->plugActionList( this, QLatin1String( "navigator_actionlist" ), mActionPlugins );
}

bool MainWindow::removePlugin( const KPluginInfo &info )
{
    PluginList::Iterator end = mPlugins.end();
    for ( PluginList::Iterator it = mPlugins.begin(); it != end; ++it ) {
        KontactInterface::Plugin *plugin = *it;
        if ( ( *it )->identifier() == info.pluginName() ) {
            QList<QAction*> actionList = plugin->newActions();
            QList<QAction*>::const_iterator listIt;
            QList<QAction*>::const_iterator listEnd( actionList.constEnd() );
            for ( listIt = actionList.constBegin(); listIt != listEnd; ++listIt ) {
                qDebug() << QLatin1String("Unplugging New actions") << (*listIt)->objectName();
                mNewActions->removeAction( *listIt );
            }

            if ( mSyncActionsEnabled ) {
                actionList = plugin->syncActions();
                for ( listIt = actionList.constBegin(); listIt != actionList.constEnd(); ++listIt ) {
                    qDebug() << QLatin1String("Unplugging Sync actions") << (*listIt)->objectName();
                    mSyncActions->removeAction( *listIt );
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
                QAction *q = mPluginAction[plugin]; // remove QAction, to free the shortcut for later use
                mActionPlugins.removeAll( q );
                mPluginAction.remove(plugin);
                delete q;
            }

            if ( mCurrentPlugin == 0 ) {
                PluginList::Iterator it;
                PluginList::Iterator pluginEnd( mPlugins.end() );
                for ( it = mPlugins.begin(); it != pluginEnd; ++it ) {
                    if ( (*it)->showInSideBar() ) {
                        selectPlugin( *it );
                        return true;
                    }
                }
            }
            return true;
        }

    }

    return false;
}

void MainWindow::addPlugin( KontactInterface::Plugin *plugin )
{
    qDebug();

    mPlugins.append( plugin );

    if ( plugin->showInSideBar() ) {
        QAction *action = new QAction( QIcon::fromTheme( plugin->icon() ), plugin->title(), this );
        //action->setHelpText(
        //            i18nc( "@info:status", "Plugin %1", plugin->title() ) );
        action->setWhatsThis(
                    i18nc( "@info:whatsthis",
                           "Switch to plugin %1", plugin->title() ) );
        action->setCheckable( true );
        action->setData( QVariant::fromValue( plugin ) ); // on the slot we can decode
        // which action was triggered
        connect( action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered()) );
        actionCollection()->addAction( plugin->title(), action );
        mActionPlugins.append( action );
        mPluginAction.insert( plugin, action );
    }

    // merge the plugins GUI into the main window
    insertChildClient( plugin );

    // sort the action plugins again and reset shortcuts. If we removed and then readded some plugins
    // we need to take in count their weights for setting shortcuts again
    qSort( mActionPlugins.begin(), mActionPlugins.end(), pluginActionWeightLessThan );
    qSort( mPlugins.begin(), mPlugins.end(), pluginWeightLessThan );
    int i = 0;
    foreach ( QAction *qaction, mActionPlugins ) {
        QAction *action = static_cast<QAction*>( qaction );
        QString shortcut = QString::fromLatin1( "Ctrl+%1" ).arg( mActionPlugins.count() - i );
        action->setShortcut( QKeySequence( shortcut ) );
        ++i;
    }
}

void MainWindow::partLoaded( KontactInterface::Plugin *plugin, KParts::ReadOnlyPart *part )
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

    qDebug() << QLatin1String("Part activated:") << part
             << QLatin1String("with stack id.")<< mPartsStack->indexOf( part->widget() );

    statusBar()->clearMessage();
}

void MainWindow::slotNewClicked()
{
    if ( !mCurrentPlugin->newActions().isEmpty() ) {
        mCurrentPlugin->newActions().first()->trigger();
    } else {
        PluginList::Iterator it;
        PluginList::Iterator end( mPlugins.end() );
        for ( it = mPlugins.begin(); it != end; ++it ) {
            if ( !(*it)->newActions().isEmpty() ) {
                (*it)->newActions().first()->trigger();
                return;
            }
        }
    }
}

void MainWindow::slotSyncClicked()
{
    if ( !mCurrentPlugin->syncActions().isEmpty() ) {
        mCurrentPlugin->syncActions().first()->trigger();
    } else {
        PluginList::Iterator it;
        PluginList::Iterator end( mPlugins.end() );
        for ( it = mPlugins.begin(); it != end; ++it ) {
            if ( !(*it)->syncActions().isEmpty() ) {
                (*it)->syncActions().first()->trigger();
                return;
            }
        }
    }
}

KToolBar *MainWindow::findToolBar( const char *name )
{
    // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
    return findChild<KToolBar *>( QLatin1String(name) );
}

void MainWindow::selectPlugin( KontactInterface::Plugin *plugin )
{
    if ( !plugin ) {
        return;
    }

    if ( plugin->isRunningStandalone() ) {
        statusBar()->showMessage(
                    i18nc( "@info:status",
                           "Application is running standalone. Foregrounding..." ), 1000 );
        plugin->bringToForeground();
        return;
    }

    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    if ( mCurrentPlugin ) {
        KConfigGroup grp = KSharedConfig::openConfig()->group(
                        QString::fromLatin1( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) );
        saveMainWindowSettings( grp );
    }

    KParts::Part *part = plugin->part();

    if ( !part ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error(
                    this,
                    i18nc( "@info",
                           "Cannot load part for %1.",
                           plugin->title() ) + QLatin1Char('\n') + lastErrorMessage() );
        plugin->setDisabled( true );
        mSidePane->updatePlugins();
        return;
    }

    if ( mCurrentPlugin ) {
        QAction *action = mPluginAction[ mCurrentPlugin ];
        if ( action ) {
            action->setChecked( false );
        }
    }
    QAction *selectedPluginAction = mPluginAction[ plugin ];
    if ( selectedPluginAction ) {
        selectedPluginAction->setChecked( true );
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

    plugin->aboutToSelect();

    mPartManager->setActivePart( part );
    QWidget *view = part->widget();
    Q_ASSERT( view );

    if ( view ) {
        mPartsStack->setCurrentWidget( view );
        view->show();

        if ( !mFocusWidgets.isEmpty() && mFocusWidgets.contains( plugin->identifier() ) ) {
            focusWidget = mFocusWidgets[ plugin->identifier() ];
            if ( focusWidget ) {
                focusWidget->setFocus();
            }
        } else {
            view->setFocus();
        }

        mCurrentPlugin = plugin;

        QAction *newAction = 0;
        if ( !plugin->newActions().isEmpty() ) {
            newAction = plugin->newActions().first();
        }

        QAction *syncAction = 0;
        if ( !plugin->syncActions().isEmpty() ) {
            syncAction = plugin->syncActions().first();
        }

        createGUI( plugin->part() );
        plugin->shortcutChanged();


        setCaption( i18nc( "@title:window Plugin dependent window title",
                           "%1 - Kontact", plugin->title() ) );

        if ( newAction ) {
            mNewActions->setIcon( newAction->icon() );
            static_cast<QAction*>( mNewActions )->setText( newAction->text() );
            mNewActions->setWhatsThis( newAction->whatsThis() );
        } else { // we'll use the action of the first plugin which offers one
            PluginList::Iterator it;
            PluginList::Iterator end(mPlugins.end());
            for ( it = mPlugins.begin(); it != end; ++it ) {
                if ( !(*it)->newActions().isEmpty() ) {
                    newAction = (*it)->newActions().first();
                }
                if ( newAction ) {
                    static_cast<QAction*>( mNewActions )->setIcon( newAction->icon() );
                    mNewActions->setText( newAction->text() );
                    mNewActions->setWhatsThis( newAction->whatsThis() );
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
                PluginList::Iterator end(mPlugins.end());
                for ( it = mPlugins.begin(); it != end; ++it ) {
                    if ( !(*it)->syncActions().isEmpty() ) {
                        syncAction = (*it)->syncActions().first();
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

    KToolBar *navigatorToolBar = findToolBar( "navigatorToolBar" );
    if ( navigatorToolBar && !navigatorToolBar->isHidden() &&
         ( toolBarArea( navigatorToolBar ) == Qt::TopToolBarArea ||
           toolBarArea( navigatorToolBar ) == Qt::BottomToolBarArea ) ) {
        addToolBar( toolBarArea( navigatorToolBar ), navigatorToolBar );
    }

    applyMainWindowSettings( KSharedConfig::openConfig()->group(
                                 QString::fromLatin1( "MainWindow%1" ).arg( plugin->identifier() ) ) );

    QApplication::restoreOverrideCursor();
}

void MainWindow::slotActionTriggered()
{
    QAction *actionSender = static_cast<QAction*>( sender() );
    actionSender->setChecked( true );
    KontactInterface::Plugin *plugin = actionSender->data().value<KontactInterface::Plugin*>();
    if ( !plugin ) {
        return;
    }
    mSidePane->setCurrentPlugin( plugin->identifier() );
}

void MainWindow::selectPlugin( const QString &pluginName )
{
    PluginList::ConstIterator end = mPlugins.constEnd();
    for ( PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it ) {
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
    PluginList::ConstIterator end( mDelayedPreload.constEnd() );
    for ( it = mDelayedPreload.constBegin(); it != end; ++it ) {
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
    PluginList::ConstIterator end = mPlugins.constEnd();
    for ( PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it ) {
        const QString file = (*it)->tipFile();
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
    static Kontact::KontactConfigureDialog *dlg = 0;
    if ( !dlg ) {
        dlg = new Kontact::KontactConfigureDialog( this );
        dlg->setAllowComponentSelection( true );

        // do not show settings of components running standalone
        KPluginInfo::List filteredPlugins = mPluginInfos;
        PluginList::ConstIterator it;
        PluginList::ConstIterator end(mPlugins.constEnd());
        for ( it = mPlugins.constBegin(); it != end; ++it ) {
            if ( (*it)->isRunningStandalone() ) {
                KPluginInfo::List::ConstIterator infoIt;
                KPluginInfo::List::ConstIterator infoEnd(filteredPlugins.constEnd());
                for ( infoIt = filteredPlugins.constBegin();
                      infoIt != infoEnd; ++infoIt ) {
                    if ( infoIt->pluginName() == (*it)->identifier() ) {
                        filteredPlugins.removeAll( *infoIt );
                        break;
                    }
                }
            }
        }

        //QT5 dlg->setHelp( QLatin1String("main-config"), QLatin1String("kontact") );
        dlg->addPluginInfos( filteredPlugins );
        connect( dlg, SIGNAL(pluginSelectionChanged()), SLOT(pluginsChanged()) );
    }

    dlg->show();
}

void MainWindow::pluginsChanged()
{
    unplugActionList( QLatin1String("navigator_actionlist") );
    unloadPlugins();
    loadPlugins();
    mSidePane->updatePlugins();
    updateShortcuts();
}

void MainWindow::updateConfig()
{
    qDebug();

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
    if ( mCurrentPlugin && mCurrentPlugin->part() ) {
        mCurrentPlugin->shortcutChanged();
    }
}

void MainWindow::configureToolbars()
{
    if ( mCurrentPlugin ) {
        KConfigGroup grp (KSharedConfig::openConfig()->group(
                        QString::fromLatin1( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
        saveMainWindowSettings( grp );
    }
    QPointer<KEditToolBar> edit = new KEditToolBar( factory() );
    connect( edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()) );
    edit->exec();
    delete edit;
}

void MainWindow::slotNewToolbarConfig()
{
    if ( mCurrentPlugin && mCurrentPlugin->part() ) {
        createGUI( mCurrentPlugin->part() );
    }
    if ( mCurrentPlugin ) {
        applyMainWindowSettings(
                    KSharedConfig::openConfig()->group(
                        QString::fromLatin1( "MainWindow%1" ).arg( mCurrentPlugin->identifier() ) ) );
    }
    updateShortcuts(); // for the plugActionList call
}

void MainWindow::slotOpenUrl( const QUrl &url )
{
    slotOpenUrl( KUrl( url ) );
}

void MainWindow::slotOpenUrl( const KUrl &url )
{
    if ( url.protocol() == QLatin1String( "exec" ) ) {
        const QString path( url.path() );
        if ( path == QLatin1String( "/switch" ) ) {
            if ( mCurrentPlugin ) {
                mPartsStack->setCurrentIndex( mPartsStack->indexOf( mCurrentPlugin->part()->widget() ) );
            }
        }
        else if ( path == QLatin1String( "/accountwizard" ) ) {
            KRun::runCommand( QLatin1String( "accountwizard" ), this );
            slotQuit();
        }
        else if ( path.startsWith( QLatin1String( "/help" ) ) ) {
            QString app( QLatin1String("kontact") );
            if ( !url.query().isEmpty() ) {
                app = url.query().mid( 1 );
            }
            KHelpClient::invokeHelp( QString(), app );
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

    if ( !activePlugins.isEmpty() ) {
        foreach ( KontactInterface::Plugin *plugin, mPlugins ) {
            if ( !plugin->isRunningStandalone() && activePlugins.contains( plugin->identifier() ) ) {
                plugin->readProperties( config );
            }
        }
    }
}

void MainWindow::saveProperties( KConfigGroup &config )
{
    Core::saveProperties( config );

    QStringList activePlugins;

    foreach ( const KPluginInfo &pluginInfo, mPluginInfos ) {
        if ( pluginInfo.isPluginEnabled() ) {
            KontactInterface::Plugin *plugin = pluginFromInfo( pluginInfo );
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

    foreach ( KontactInterface::Plugin *plugin, mPlugins ) {
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
    const int iconSize = iconloader->currentSize( KIconLoader::Desktop );

    const QString handbook_icon_path = iconloader->iconPath( QLatin1String("help-contents"), KIconLoader::Desktop );
    const QString html_icon_path = iconloader->iconPath( QLatin1String("kontact"), KIconLoader::Desktop );
    const QString wizard_icon_path = iconloader->iconPath( QLatin1String("tools-wizard"), KIconLoader::Desktop );

    QString info =
            ki18nc(
                "@info",
                "<h2 style='text-align:center; margin-top: 0px;'>Welcome to Kontact %1</h2>"
                "<p align=\"center\">%2</p>"
                "<table align=\"center\">"
                "<tr><td><a href=\"%3\"><img width=\"%4\" height=\"%5\" src=\"%6\" /></a></td>"
                "<td><a href=\"%7\">%8</a><br /><span id=\"subtext\"><nobr>%9</nobr></span></td></tr>"
                "<tr><td><a href=\"%10\"><img width=\"%11\" height=\"%12\" src=\"%13\" /></a></td>"
                "<td><a href=\"%14\">%15</a><br /><span id=\"subtext\"><nobr>%16</nobr></span></td></tr>"
                "<tr><td><a href=\"%17\"><img width=\"%18\" height=\"%19\" src=\"%20\" /></a></td>"
                "<td><a href=\"%21\">%22</a><br /><span id=\"subtext\"><nobr>%23</nobr></span></td></tr>"
                "</table>"
                "<p style=\"margin-bottom: 0px\"> <a href=\"%24\">Skip this introduction</a></p>" ).
            subs( KComponentData::mainComponent().aboutData()->version() ).
            subs( i18nc( "@item:intext",
                         "Kontact handles your e-mail, address book, calendar, to-do list and more." ) ).
            subs( QLatin1String("exec:/help?kontact") ).
            subs( iconSize ).
            subs( iconSize ).
            subs( QLatin1String("file:") + handbook_icon_path ).
            subs( QLatin1String("exec:/help?kontact") ).
            subs( i18nc( "@item:intext", "Read Manual" ) ).
            subs( i18nc( "@item:intext", "Learn more about Kontact and its components" ) ).
            subs( QLatin1String("http://kontact.org") ).
            subs( iconSize ).
            subs( iconSize ).
            subs( QLatin1String("file:") + html_icon_path ).
            subs( QLatin1String("http://kontact.org") ).
            subs( i18nc( "@item:intext", "Visit Kontact Website" ) ).
            subs( i18nc( "@item:intext", "Access online resources and tutorials" ) ).
            subs( QLatin1String("exec:/accountwizard") ).
            subs( iconSize ).
            subs( iconSize ).
            subs( QLatin1String("file:") + wizard_icon_path ).
            subs( QLatin1String("exec:/accountwizard") ).
            subs( i18nc( "@item:intext", "Setup your Accounts" ) ).
            subs( i18nc( "@item:intext", "Prepare Kontact for use" ) ).
            subs( QLatin1String("exec:/switch") ).
            toString();
    return info;
}

void MainWindow::slotShowHideSideBar()
{
    QList<int> sizes = mSplitter->sizes();
    if (!sizes.isEmpty()) {
        if (sizes.at(0) != 0) {
            sizes[0] = 0;
        } else {
            sizes[0] = 10;
        }
        mSplitter->setSizes(sizes);
    }
}

