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

#include <Libkdepim/BroadcastStatus>
#include <Libkdepim/ProgressStatusBarWidget>
#include <Libkdepim/StatusbarProgressWidget>

#include <KontactInterface/Core>

#include <QStatusBar>
#include <KWindowConfig>
#include <KXMLGUIFactory>
#include <KActionCollection>
#include <KActionMenu>
#include <KConfigGroup>
#include <KDBusServiceStarter>
#include "kontact_debug.h"
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
#include <KToolBar>
#include <KParts/PartManager>
#include <ksettings/Dispatcher>
#include <KSycoca>
#include <KIconLoader>
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
#include <QFile>
#include <QHBoxLayout>
#include <QApplication>
#include <KAboutData>
#include <QFontDatabase>

#include <grantleetheme/grantleethememanager.h>
#include <grantleetheme/grantleetheme.h>

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

    virtual int startServiceFor(const QString &serviceType,
                                const QString &constraint = QString(),
                                QString *error = Q_NULLPTR, QString *dbusService = Q_NULLPTR,
                                int flags = 0) Q_DECL_OVERRIDE;

    // We need to keep track of the plugins which are loaded, so pass a pointer
    // to the plugin list here. Be sure to reset it back to 0 with
    // setPluginList() as soon as the list gets destroyed.
    ServiceStarter(PluginList *pluginList)
    {
        mPlugins = pluginList;
    }

    static void setPluginList(PluginList *pluginList)
    {
        mPlugins = pluginList;
    }

protected:

    virtual ~ServiceStarter() {}
    static PluginList *mPlugins;
};

PluginList *ServiceStarter::mPlugins = Q_NULLPTR;

int ServiceStarter::startServiceFor(const QString &serviceType,
                                    const QString &constraint,
                                    QString *error, QString *dbusService,
                                    int flags)
{
    if (mPlugins) {
        PluginList::ConstIterator end = mPlugins->constEnd();
        for (PluginList::ConstIterator it = mPlugins->constBegin(); it != end; ++it) {
            if ((*it)->createDBUSInterface(serviceType)) {
                qCDebug(KONTACT_LOG) << "found interface for" << serviceType;
                if (dbusService) {
                    *dbusService = (*it)->registerClient();
                }
                return 0;
            }
        }
    }

    qCDebug(KONTACT_LOG) << "Didn't find dbus interface, falling back to external process";
    return KDBusServiceStarter::startServiceFor(serviceType, constraint,
            error, dbusService, flags);
}

MainWindow::MainWindow()
    : KontactInterface::Core(), mSplitter(Q_NULLPTR), mCurrentPlugin(Q_NULLPTR), mAboutDialog(Q_NULLPTR),
      mReallyClose(false), mSyncActionsEnabled(true)
{
    // The ServiceStarter created here will be deleted by the KDbusServiceStarter
    // base class, which is a global static.
    new ServiceStarter(&mPlugins);

    QDBusConnection::sessionBus().registerObject(
        QStringLiteral("/KontactInterface"), this, QDBusConnection::ExportScriptableSlots);

    // Set this to be the group leader for all subdialogs - this means
    // modal subdialogs will only affect this dialog, not the other windows
    setAttribute(Qt::WA_GroupLeader);

    initGUI();
    initObject();

    mSidePane->setMaximumWidth(mSidePane->sizeHint().width());
    mSidePane->setMinimumWidth(mSidePane->sizeHint().width());

    factory()->plugActionList(this, QStringLiteral("navigator_actionlist"), mActionPlugins);

    KConfigGroup grp(KSharedConfig::openConfig(), "MainWindow");
    KWindowConfig::restoreWindowSize(windowHandle(), grp);
    setAutoSaveSettings();
}

void MainWindow::initGUI()
{
    initWidgets();
    setupActions();
    setHelpMenuEnabled(false);
    KHelpMenu *helpMenu = new KHelpMenu(this, QString(), true/*, actionCollection() QT5*/);
    connect(helpMenu, &KHelpMenu::showAboutApplication, this, &MainWindow::showAboutDialog);

    KStandardAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());
    setXMLFile(QStringLiteral("kontactui.rc"));

    setStandardToolBarMenuEnabled(true);

    createGUI(Q_NULLPTR);

    KToolBar *navigatorToolBar = findToolBar("navigatorToolBar");
    if (navigatorToolBar) {
        if (layoutDirection() == Qt::LeftToRight) {
            navigatorToolBar->setLayoutDirection(Qt::RightToLeft);
        } else {
            navigatorToolBar->setLayoutDirection(Qt::LeftToRight);
        }
        Q_ASSERT(navigatorToolBar->sizeHint().isValid());
        navigatorToolBar->setMinimumWidth(navigatorToolBar->sizeHint().width());
    } else {
        qCritical() << "Unable to find navigatorToolBar, probably kontactui.rc is missing";
    }
}

void MainWindow::waitForKSycoca()
{
    int i = 0;
    while (i < KSYCOCA_WAIT_TIMEOUT) {
        if (KSycoca::isAvailable()) {
            return;
        }
        // When KSycoca is not availabe that usually means Kontact
        // was started before kded is done with it's first run
        // we want to block Kontact execution to
        // give Kded time to initialize and create the
        // System Configuration database necessary for further
        // Kontact startup
        qCDebug(KONTACT_LOG) << "Waiting for KSycoca";
        sleep(1);
        ++i;
    }
    // This should only happen if the distribution is broken
    qFatal("KSycoca unavailable. Kontact will be unable to find plugins.");
}

void MainWindow::initObject()
{
    if (!KSycoca::isAvailable()) {
        waitForKSycoca();
    }
    KService::List offers = KServiceTypeTrader::self()->query(
                                QStringLiteral("Kontact/Plugin"),
                                QStringLiteral("[X-KDE-KontactPluginVersion] == %1").arg(KONTACT_PLUGIN_VERSION));
    mPluginInfos = KPluginInfo::fromServices(
                       offers, KConfigGroup(Prefs::self()->config(), "Plugins"));
    KPluginInfo::List::Iterator it;
    KPluginInfo::List::Iterator end(mPluginInfos.end());

    for (it = mPluginInfos.begin(); it != end; ++it) {
        it->load();
    }

    // prepare the part manager
    mPartManager = new KParts::PartManager(this);
    connect(mPartManager, &KParts::PartManager::activePartChanged, this, &MainWindow::slotActivePartChanged);

    loadPlugins();

    if (mSidePane) {
        mSidePane->updatePlugins();
    }
    KSettings::Dispatcher::registerComponent(QStringLiteral("kontact"), this, "updateConfig");

    loadSettings();

    statusBar()->show();

    // done initializing
    slotShowStatusMsg(QString());

    connect(KPIM::BroadcastStatus::instance(), SIGNAL(statusMsg(QString)), this, SLOT(slotShowStatusMsg(QString)));

    // launch commandline specified module if any
    activateInitialPluginModule();

    if (Prefs::lastVersionSeen() == KAboutData::applicationData().version()) {
        selectPlugin(mCurrentPlugin);
    }

    paintAboutScreen(QStringLiteral("introduction_kontact.html"), introductionData());
    Prefs::setLastVersionSeen(KAboutData::applicationData().version());
}

MainWindow::~MainWindow()
{
    if (mCurrentPlugin) {
        KConfigGroup grp(
            KSharedConfig::openConfig()->group(
                QStringLiteral("MainWindow%1").arg(mCurrentPlugin->identifier())));
        saveMainWindowSettings(grp);
    }

    createGUI(Q_NULLPTR);
    ServiceStarter::setPluginList(Q_NULLPTR);
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
    for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
        delete *it;
    }
}

// Called by main().
void MainWindow::setInitialActivePluginModule(const QString &module)
{
    if (mInitialActiveModule != module) {
        mInitialActiveModule = module;
        activateInitialPluginModule();
    }
}

bool MainWindow::pluginActionWeightLessThan(const QAction *left, const QAction *right)
{
    // Since this lessThan method is used only for the toolbar (which is on
    // the inverse layout direction than the rest of the system), we add the
    // elements on the exactly inverse order. (ereslibre)
    return !pluginWeightLessThan(left->data().value<KontactInterface::Plugin *>(),
                                 right->data().value<KontactInterface::Plugin *>());
}

bool MainWindow::pluginWeightLessThan(const KontactInterface::Plugin *left,
                                      const KontactInterface::Plugin *right)
{
    return left->weight() < right->weight();
}

void MainWindow::activateInitialPluginModule()
{
    if (!mInitialActiveModule.isEmpty() && !mPlugins.isEmpty()) {
        PluginList::ConstIterator end = mPlugins.constEnd();
        for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
            if (!(*it)->identifier().isEmpty() &&
                    (*it)->identifier().contains(mInitialActiveModule)) {
                selectPlugin(*it);
                return;
            }
        }
    }
}

void MainWindow::initWidgets()
{
    QWidget *mTopWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    mTopWidget->setLayout(layout);
    setCentralWidget(mTopWidget);

    mSplitter = new QSplitter(mTopWidget);
    layout->addWidget(mSplitter);
    mSidePane = new IconSidePane(this, mSplitter);
    /*
    // don't occupy screen estate on load

    QList<int> sizes;
    sizes << 0;
    mSplitter->setSizes(sizes);
    */
    connect(mSidePane, SIGNAL(pluginSelected(KontactInterface::Plugin*)), SLOT(selectPlugin(KontactInterface::Plugin*)));

    mPartsStack = new QStackedWidget(mSplitter);
    mPartsStack->layout()->setSpacing(0);

    initAboutScreen();

    paintAboutScreen(QStringLiteral("loading_kontact.html"), QVariantHash());

    KPIM::ProgressStatusBarWidget *progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    mStatusMsgLabel =
        new KSqueezedTextLabel(i18nc("@info:status", " Initializing..."), statusBar());
    mStatusMsgLabel->setTextElideMode(Qt::ElideRight);
    mStatusMsgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    statusBar()->addWidget(mStatusMsgLabel, 10);
    statusBar()->addPermanentWidget(progressStatusBarWidget->littleProgress(), 0);

    mSplitter->setCollapsible(1, false);
}

void MainWindow::paintAboutScreen(const QString &templateName, const QVariantHash &data)
{
    GrantleeTheme::ThemeManager manager(QStringLiteral("splashPage"),
                                        QStringLiteral("splash.theme"),
                                        Q_NULLPTR,
                                        QStringLiteral("messageviewer/about/"));
    GrantleeTheme::Theme theme = manager.theme(QStringLiteral("default"));
    if (!theme.isValid()) {
        qCDebug(KONTACT_LOG) << "Theme error: failed to find splash theme";
    } else {
        mIntroPart->setHtml(theme.render(templateName, data),
                            QUrl::fromLocalFile(theme.absolutePath() + QLatin1Char('/')));
    }

}

void MainWindow::initAboutScreen()
{
    QWidget *introbox = new QWidget(mPartsStack);
    QHBoxLayout *introboxHBoxLayout = new QHBoxLayout(introbox);
    introboxHBoxLayout->setMargin(0);
    mPartsStack->addWidget(introbox);
    mPartsStack->setCurrentWidget(introbox);
    mIntroPart = new KWebView(introbox);
    introboxHBoxLayout->addWidget(mIntroPart);
    mIntroPart->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    mIntroPart->setFocusPolicy(Qt::WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    mIntroPart->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    mIntroPart->settings()->setAttribute(QWebSettings::JavaEnabled, false);
    mIntroPart->settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    const QFontInfo font(QFontDatabase().systemFont(QFontDatabase::GeneralFont));
    mIntroPart->settings()->setFontFamily(QWebSettings::StandardFont, font.family());
    mIntroPart->settings()->setFontSize(QWebSettings::DefaultFontSize, font.pixelSize());


    connect(mIntroPart->page(), &QWebPage::linkClicked, this, &MainWindow::slotOpenUrl, Qt::QueuedConnection);
}

void MainWindow::setupActions()
{
    actionCollection()->addAction(KStandardAction::Quit, this, SLOT(slotQuit()));

    mNewActions = new KActionMenu(
        i18nc("@title:menu create new pim items (message,calendar,to-do,etc.)", "New"), this);
    actionCollection()->addAction(QStringLiteral("action_new"), mNewActions);
    actionCollection()->setDefaultShortcuts(mNewActions, KStandardShortcut::openNew());
    connect(mNewActions, &KActionMenu::triggered, this, &MainWindow::slotNewClicked);

    // If the user is using disconnected imap mail folders as groupware, we add
    // plugins' Synchronize actions to the toolbar which trigger an imap sync.
    // Otherwise it's redundant and misleading.
    KConfig _config(QStringLiteral("kmail2rc"));
    KConfigGroup config(&_config, "Groupware");
#if defined(KDEPIM_ENTERPRISE_BUILD)
    bool defGW = config.readEntry("Enabled", true);
#else
    bool defGW = config.readEntry("Enabled", false);
#endif
    KConfig *_cfg = Prefs::self()->config();
    KConfigGroup cfg(_cfg, "Kontact Groupware Settings");
    mSyncActionsEnabled = cfg.readEntry("GroupwareMailFoldersEnabled", defGW);

    if (mSyncActionsEnabled) {
        mSyncActions = new KActionMenu(
            QIcon::fromTheme(QStringLiteral("view-refresh")),
            i18nc("@title:menu synchronize pim items (message,calendar,to-do,etc.)", "Sync"), this);
        actionCollection()->addAction(QStringLiteral("action_sync"), mSyncActions);
        actionCollection()->setDefaultShortcuts(mSyncActions, KStandardShortcut::reload());
        connect(mSyncActions, &KActionMenu::triggered, this, &MainWindow::slotSyncClicked);
    }

    QAction *action =
        new QAction(QIcon::fromTheme(QStringLiteral("configure")),
                    i18nc("@action:inmenu", "Configure Kontact..."), this);
    setHelpText(action, i18nc("@info:status", "Configure Kontact"));
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can configure Kontact."));
    actionCollection()->addAction(QStringLiteral("settings_configure_kontact"), action);
    connect(action, &QAction::triggered, this, &MainWindow::slotPreferences);

    action =
        new QAction(QIcon::fromTheme(QStringLiteral("kontact")),
                    i18nc("@action:inmenu", "&Kontact Introduction"), this);
    setHelpText(action, i18nc("@info:status", "Show the Kontact Introduction page"));
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to see the Kontact Introduction page."));
    actionCollection()->addAction(QStringLiteral("help_introduction"), action);
    connect(action, &QAction::triggered, this, &MainWindow::slotShowIntroduction);

    //TODO 4.12: add description
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F9), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::slotShowHideSideBar);
}

bool MainWindow::isPluginLoaded(const KPluginInfo &info)
{
    return (pluginFromInfo(info) != Q_NULLPTR);
}

KontactInterface::Plugin *MainWindow::pluginFromInfo(const KPluginInfo &info)
{
    PluginList::ConstIterator end = mPlugins.constEnd();
    for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
        if ((*it)->identifier() == info.pluginName()) {
            return *it;
        }
    }
    return Q_NULLPTR;
}

void MainWindow::loadPlugins()
{
    QList<KontactInterface::Plugin *> plugins;

    int i;
    KPluginInfo::List::ConstIterator it;
    KPluginInfo::List::ConstIterator end(mPluginInfos.constEnd());
    for (it = mPluginInfos.constBegin(); it != end; ++it) {
        if (!it->isPluginEnabled()) {
            continue;
        }

        KontactInterface::Plugin *plugin = Q_NULLPTR;
        if (isPluginLoaded(*it)) {
            plugin = pluginFromInfo(*it);
            if (plugin) {
                plugin->configUpdated();
            }
            continue;
        }

        qCDebug(KONTACT_LOG) << "Loading Plugin:" << it->name();
        QString error;
        plugin =
            it->service()->createInstance<KontactInterface::Plugin>(this, QVariantList(), &error);

        if (!plugin) {
            qCDebug(KONTACT_LOG) << "Unable to create plugin for" << it->name() << error;
            continue;
        }

        plugin->setIdentifier(it->pluginName());
        plugin->setTitle(it->name());
        plugin->setIcon(it->icon());

        QVariant libNameProp = it->property(QStringLiteral("X-KDE-KontactPartLibraryName"));
        QVariant exeNameProp = it->property(QStringLiteral("X-KDE-KontactPartExecutableName"));
        QVariant loadOnStart = it->property(QStringLiteral("X-KDE-KontactPartLoadOnStart"));
        QVariant hasPartProp = it->property(QStringLiteral("X-KDE-KontactPluginHasPart"));

        if (!loadOnStart.isNull() && loadOnStart.toBool()) {
            mDelayedPreload.append(plugin);
        }

        qCDebug(KONTACT_LOG) << "LIBNAMEPART:" << libNameProp.toString();

        plugin->setPartLibraryName(libNameProp.toString().toUtf8());
        plugin->setExecutableName(exeNameProp.toString());
        if (hasPartProp.isValid()) {
            plugin->setShowInSideBar(hasPartProp.toBool());
        }

        for (i = 0; i < plugins.count(); ++i) {
            KontactInterface::Plugin *p = plugins.at(i);
            if (plugin->weight() < p->weight()) {
                break;
            }
        }
        plugins.insert(i, plugin);

    }

    const int numberOfPlugins(plugins.count());
    for (i = 0; i < numberOfPlugins; ++i) {
        KontactInterface::Plugin *plugin = plugins.at(i);

        const QList<QAction *> actionList = plugin->newActions();
        QList<QAction *>::const_iterator listIt;
        QList<QAction *>::const_iterator end(actionList.end());

        for (listIt = actionList.begin(); listIt != end; ++listIt) {
            qCDebug(KONTACT_LOG) << QStringLiteral("Plugging New actions") << (*listIt)->objectName();
            mNewActions->addAction((*listIt));
        }

        if (mSyncActionsEnabled) {
            Q_FOREACH (QAction *listIt, plugin->syncActions()) {
                qCDebug(KONTACT_LOG) << QStringLiteral("Plugging Sync actions") << listIt->objectName();
                mSyncActions->addAction(listIt);
            }
        }
        addPlugin(plugin);
    }

    const bool state = (!mPlugins.isEmpty());
    mNewActions->setEnabled(state);
    if (mSyncActionsEnabled) {
        mSyncActions->setEnabled(state);
    }
}

void MainWindow::unloadPlugins()
{
    KPluginInfo::List::ConstIterator end = mPluginInfos.constEnd();
    KPluginInfo::List::ConstIterator it;
    for (it = mPluginInfos.constBegin(); it != end; ++it) {
        if (!it->isPluginEnabled()) {
            removePlugin(*it);
        }
    }
}

void MainWindow::updateShortcuts()
{
    ActionPluginList::ConstIterator end = mActionPlugins.constEnd();
    ActionPluginList::ConstIterator it;
    int i = 0;
    for (it = mActionPlugins.constBegin(); it != end; ++it) {
        QAction *action = static_cast<QAction *>(*it);
        const QString shortcut = QStringLiteral("Ctrl+%1").arg(mActionPlugins.count() - i);
        actionCollection()->setDefaultShortcut(action, QKeySequence(shortcut));
        ++i;
    }
    factory()->plugActionList(this, QStringLiteral("navigator_actionlist"), mActionPlugins);
}

bool MainWindow::removePlugin(const KPluginInfo &info)
{
    PluginList::Iterator end = mPlugins.end();
    for (PluginList::Iterator it = mPlugins.begin(); it != end; ++it) {
        KontactInterface::Plugin *plugin = *it;
        if ((*it)->identifier() == info.pluginName()) {
            QList<QAction *> actionList = plugin->newActions();
            QList<QAction *>::const_iterator listIt;
            QList<QAction *>::const_iterator listEnd(actionList.constEnd());
            for (listIt = actionList.constBegin(); listIt != listEnd; ++listIt) {
                qCDebug(KONTACT_LOG) << QStringLiteral("Unplugging New actions") << (*listIt)->objectName();
                mNewActions->removeAction(*listIt);
            }

            if (mSyncActionsEnabled) {
                actionList = plugin->syncActions();
                for (listIt = actionList.constBegin(); listIt != actionList.constEnd(); ++listIt) {
                    qCDebug(KONTACT_LOG) << QStringLiteral("Unplugging Sync actions") << (*listIt)->objectName();
                    mSyncActions->removeAction(*listIt);
                }
            }
            removeChildClient(plugin);

            if (mCurrentPlugin == plugin) {
                mCurrentPlugin = Q_NULLPTR;
                createGUI(Q_NULLPTR);
            }

            plugin->deleteLater(); // removes the part automatically
            mPlugins.erase(it);
            if (plugin->showInSideBar()) {
                QAction *q = mPluginAction[plugin]; // remove QAction, to free the shortcut for later use
                mActionPlugins.removeAll(q);
                mPluginAction.remove(plugin);
                delete q;
            }

            if (mCurrentPlugin == Q_NULLPTR) {
                PluginList::Iterator it;
                PluginList::Iterator pluginEnd(mPlugins.end());
                for (it = mPlugins.begin(); it != pluginEnd; ++it) {
                    if ((*it)->showInSideBar()) {
                        selectPlugin(*it);
                        return true;
                    }
                }
            }
            return true;
        }

    }

    return false;
}

void MainWindow::addPlugin(KontactInterface::Plugin *plugin)
{
    qCDebug(KONTACT_LOG);

    mPlugins.append(plugin);

    if (plugin->showInSideBar()) {
        QAction *action = new QAction(QIcon::fromTheme(plugin->icon()), plugin->title(), this);
        //action->setHelpText(
        //            i18nc( "@info:status", "Plugin %1", plugin->title() ) );
        action->setWhatsThis(
            i18nc("@info:whatsthis",
                  "Switch to plugin %1", plugin->title()));
        action->setCheckable(true);
        action->setData(QVariant::fromValue(plugin));     // on the slot we can decode
        // which action was triggered
        connect(action, &QAction::triggered, this, &MainWindow::slotActionTriggered);
        actionCollection()->addAction(plugin->title(), action);
        mActionPlugins.append(action);
        mPluginAction.insert(plugin, action);
    }

    // merge the plugins GUI into the main window
    insertChildClient(plugin);

    // sort the action plugins again and reset shortcuts. If we removed and then readded some plugins
    // we need to take in count their weights for setting shortcuts again
    qSort(mActionPlugins.begin(), mActionPlugins.end(), pluginActionWeightLessThan);
    qSort(mPlugins.begin(), mPlugins.end(), pluginWeightLessThan);
    int i = 0;
    foreach (QAction *qaction, mActionPlugins) {
        QAction *action = static_cast<QAction *>(qaction);
        QString shortcut = QStringLiteral("Ctrl+%1").arg(mActionPlugins.count() - i);
        actionCollection()->setDefaultShortcut(action, QKeySequence(shortcut));
        ++i;
    }
}

void MainWindow::partLoaded(KontactInterface::Plugin *plugin, KParts::ReadOnlyPart *part)
{
    Q_UNUSED(plugin);

    // See if we have this part already (e.g. due to two plugins sharing it)
    if (mPartsStack->indexOf(part->widget()) != -1) {
        return;
    }

    mPartsStack->addWidget(part->widget());

    mPartManager->addPart(part, false);
    // Workaround for KParts misbehavior: addPart calls show!
    part->widget()->hide();
}

void MainWindow::slotActivePartChanged(KParts::Part *part)
{
    if (!part) {
        createGUI(Q_NULLPTR);
        return;
    }

    qCDebug(KONTACT_LOG) << QStringLiteral("Part activated:") << part
                         << QStringLiteral("with stack id.") << mPartsStack->indexOf(part->widget());

    statusBar()->clearMessage();
}

void MainWindow::slotNewClicked()
{
    if (!mCurrentPlugin->newActions().isEmpty()) {
        mCurrentPlugin->newActions().at(0)->trigger();
    } else {
        PluginList::Iterator it;
        PluginList::Iterator end(mPlugins.end());
        for (it = mPlugins.begin(); it != end; ++it) {
            if (!(*it)->newActions().isEmpty()) {
                (*it)->newActions().first()->trigger();
                return;
            }
        }
    }
}

void MainWindow::slotSyncClicked()
{
    if (!mCurrentPlugin->syncActions().isEmpty()) {
        mCurrentPlugin->syncActions().at(0)->trigger();
    } else {
        PluginList::Iterator it;
        PluginList::Iterator end(mPlugins.end());
        for (it = mPlugins.begin(); it != end; ++it) {
            if (!(*it)->syncActions().isEmpty()) {
                (*it)->syncActions().first()->trigger();
                return;
            }
        }
    }
}

KToolBar *MainWindow::findToolBar(const char *name)
{
    // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
    return findChild<KToolBar *>(QLatin1String(name));
}

void MainWindow::selectPlugin(KontactInterface::Plugin *plugin)
{
    if (!plugin) {
        return;
    }

    if (plugin->isRunningStandalone()) {
        statusBar()->showMessage(
            i18nc("@info:status",
                  "Application is running standalone. Foregrounding..."), 1000);
        plugin->bringToForeground();
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (mCurrentPlugin) {
        KConfigGroup grp = KSharedConfig::openConfig()->group(
                               QStringLiteral("MainWindow%1").arg(mCurrentPlugin->identifier()));
        saveMainWindowSettings(grp);
    }

    KParts::Part *part = plugin->part();

    if (!part) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error(
            this,
            i18nc("@info",
                  "Cannot load part for %1.",
                  plugin->title()) + QLatin1Char('\n') + lastErrorMessage());
        plugin->setDisabled(true);
        mSidePane->updatePlugins();
        return;
    }

    if (mCurrentPlugin) {
        QAction *action = mPluginAction[ mCurrentPlugin ];
        if (action) {
            action->setChecked(false);
        }
    }
    QAction *selectedPluginAction = mPluginAction[ plugin ];
    if (selectedPluginAction) {
        selectedPluginAction->setChecked(true);
    }

    // store old focus widget
    QWidget *focusWidget = qApp->focusWidget();
    if (mCurrentPlugin && focusWidget) {
        // save the focus widget only when it belongs to the activated part
        QWidget *parent = focusWidget->parentWidget();
        while (parent) {
            if (parent == mCurrentPlugin->part()->widget()) {
                mFocusWidgets.insert(mCurrentPlugin->identifier(), QPointer<QWidget>(focusWidget));
            }
            parent = parent->parentWidget();
        }
    }

    if (mSidePane) {
        mSidePane->setCurrentPlugin(plugin->identifier());
    }

    plugin->aboutToSelect();

    mPartManager->setActivePart(part);
    QWidget *view = part->widget();
    Q_ASSERT(view);

    if (view) {
        mPartsStack->setCurrentWidget(view);
        view->show();

        if (!mFocusWidgets.isEmpty() && mFocusWidgets.contains(plugin->identifier())) {
            focusWidget = mFocusWidgets[ plugin->identifier() ];
            if (focusWidget) {
                focusWidget->setFocus();
            }
        } else {
            view->setFocus();
        }

        mCurrentPlugin = plugin;

        QAction *newAction = Q_NULLPTR;
        if (!plugin->newActions().isEmpty()) {
            newAction = plugin->newActions().first();
        }

        QAction *syncAction = Q_NULLPTR;
        if (!plugin->syncActions().isEmpty()) {
            syncAction = plugin->syncActions().at(0);
        }

        createGUI(plugin->part());
        plugin->shortcutChanged();

        setCaption(i18nc("@title:window Plugin dependent window title",
                         "%1 - Kontact", plugin->title()));

        if (newAction) {
            mNewActions->setIcon(newAction->icon());
            static_cast<QAction *>(mNewActions)->setText(newAction->text());
            mNewActions->setWhatsThis(newAction->whatsThis());
        } else { // we'll use the action of the first plugin which offers one
            PluginList::Iterator it;
            PluginList::Iterator end(mPlugins.end());
            for (it = mPlugins.begin(); it != end; ++it) {
                if (!(*it)->newActions().isEmpty()) {
                    newAction = (*it)->newActions().first();
                }
                if (newAction) {
                    static_cast<QAction *>(mNewActions)->setIcon(newAction->icon());
                    mNewActions->setText(newAction->text());
                    mNewActions->setWhatsThis(newAction->whatsThis());
                    break;
                }
            }
        }

        if (mSyncActionsEnabled) {
            if (syncAction) {
                mSyncActions->setIcon(syncAction->icon());
                static_cast<QAction *>(mSyncActions)->setText(syncAction->text());
            } else { // we'll use the action of the first plugin which offers one
                PluginList::Iterator it;
                PluginList::Iterator end(mPlugins.end());
                for (it = mPlugins.begin(); it != end; ++it) {
                    if (!(*it)->syncActions().isEmpty()) {
                        syncAction = (*it)->syncActions().first();
                    }
                    if (syncAction) {
                        static_cast<QAction *>(mSyncActions)->setIcon(syncAction->icon());
                        mSyncActions->setText(syncAction->text());
                        break;
                    }
                }
            }
        }
    }

    KToolBar *navigatorToolBar = findToolBar("navigatorToolBar");
    if (navigatorToolBar && !navigatorToolBar->isHidden() &&
            (toolBarArea(navigatorToolBar) == Qt::TopToolBarArea ||
             toolBarArea(navigatorToolBar) == Qt::BottomToolBarArea)) {
        addToolBar(toolBarArea(navigatorToolBar), navigatorToolBar);
    }

    applyMainWindowSettings(KSharedConfig::openConfig()->group(
                                QStringLiteral("MainWindow%1").arg(plugin->identifier())));

    QApplication::restoreOverrideCursor();
}

void MainWindow::slotActionTriggered()
{
    QAction *actionSender = static_cast<QAction *>(sender());
    actionSender->setChecked(true);
    KontactInterface::Plugin *plugin = actionSender->data().value<KontactInterface::Plugin *>();
    if (!plugin) {
        return;
    }
    mSidePane->setCurrentPlugin(plugin->identifier());
}

void MainWindow::selectPlugin(const QString &pluginName)
{
    PluginList::ConstIterator end = mPlugins.constEnd();
    for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
        if ((*it)->identifier() == pluginName) {
            selectPlugin(*it);
            return;
        }
    }
}

void MainWindow::loadSettings()
{
    if (mSplitter) {
        // if the preferences do not contain useful values, the side pane part of the splitter
        // takes up the full width of the window, so leave the splitter sizing at the widget defaults
        QList<int> sizes = Prefs::self()->sidePaneSplitter();
        if (sizes.count() == mSplitter->count()) {
            mSplitter->setSizes(sizes);
        }
    }

    // Preload Plugins. This _must_ happen before the default part is loaded
    PluginList::ConstIterator it;
    PluginList::ConstIterator end(mDelayedPreload.constEnd());
    for (it = mDelayedPreload.constBegin(); it != end; ++it) {
        selectPlugin(*it);
    }
    selectPlugin(Prefs::self()->mActivePlugin);
}

void MainWindow::saveSettings()
{
    if (mSplitter) {
        Prefs::self()->mSidePaneSplitter = mSplitter->sizes();
    }

    if (mCurrentPlugin) {
        Prefs::self()->mActivePlugin = mCurrentPlugin->identifier();
    }
}

void MainWindow::slotShowIntroduction()
{
    mPartsStack->setCurrentIndex(0);
}

void MainWindow::slotQuit()
{
    mReallyClose = true;
    close();
}

void MainWindow::slotPreferences()
{
    static Kontact::KontactConfigureDialog *dlg = Q_NULLPTR;
    if (!dlg) {
        dlg = new Kontact::KontactConfigureDialog(this);
        dlg->setAllowComponentSelection(true);

        // do not show settings of components running standalone
        KPluginInfo::List filteredPlugins = mPluginInfos;
        PluginList::ConstIterator it;
        PluginList::ConstIterator end(mPlugins.constEnd());
        for (it = mPlugins.constBegin(); it != end; ++it) {
            if ((*it)->isRunningStandalone()) {
                KPluginInfo::List::ConstIterator infoIt;
                KPluginInfo::List::ConstIterator infoEnd(filteredPlugins.constEnd());
                for (infoIt = filteredPlugins.constBegin();
                        infoIt != infoEnd; ++infoIt) {
                    if (infoIt->pluginName() == (*it)->identifier()) {
                        filteredPlugins.removeAll(*infoIt);
                        break;
                    }
                }
            }
        }
        dlg->addPluginInfos(filteredPlugins);
        connect(dlg, &Kontact::KontactConfigureDialog::pluginSelectionChanged, this, &MainWindow::pluginsChanged);
    }

    dlg->show();
}

void MainWindow::pluginsChanged()
{
    unplugActionList(QStringLiteral("navigator_actionlist"));
    unloadPlugins();
    loadPlugins();
    mSidePane->updatePlugins();
    updateShortcuts();
}

void MainWindow::updateConfig()
{
    qCDebug(KONTACT_LOG);

    saveSettings();
    loadSettings();
}

void MainWindow::showAboutDialog()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (!mAboutDialog) {
        mAboutDialog = new AboutDialog(this);
    }

    mAboutDialog->show();
    mAboutDialog->raise();
    QApplication::restoreOverrideCursor();
}

void MainWindow::configureShortcuts()
{
    KShortcutsDialog dialog(
        KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection());

    if (mCurrentPlugin && mCurrentPlugin->part()) {
        dialog.addCollection(mCurrentPlugin->part()->actionCollection());
    }

    dialog.configure();
    if (mCurrentPlugin && mCurrentPlugin->part()) {
        mCurrentPlugin->shortcutChanged();
    }
}

void MainWindow::configureToolbars()
{
    if (mCurrentPlugin) {
        KConfigGroup grp(KSharedConfig::openConfig()->group(
                             QStringLiteral("MainWindow%1").arg(mCurrentPlugin->identifier())));
        saveMainWindowSettings(grp);
    }
    QPointer<KEditToolBar> edit = new KEditToolBar(factory());
    connect(edit.data(), &KEditToolBar::newToolBarConfig, this, &MainWindow::slotNewToolbarConfig);
    edit->exec();
    delete edit;
}

void MainWindow::slotNewToolbarConfig()
{
    if (mCurrentPlugin && mCurrentPlugin->part()) {
        createGUI(mCurrentPlugin->part());
    }
    if (mCurrentPlugin) {
        applyMainWindowSettings(
            KSharedConfig::openConfig()->group(
                QStringLiteral("MainWindow%1").arg(mCurrentPlugin->identifier())));
    }
    updateShortcuts(); // for the plugActionList call
}

void MainWindow::slotOpenUrl(const QUrl &url)
{
    if (url.scheme() == QLatin1String("exec")) {
        const QString path(url.path());
        if (path == QLatin1String("/switch")) {
            if (mCurrentPlugin) {
                mPartsStack->setCurrentIndex(mPartsStack->indexOf(mCurrentPlugin->part()->widget()));
            }
        } else if (path == QLatin1String("/accountwizard")) {
            KRun::runCommand(QStringLiteral("accountwizard"), this);
            slotQuit();
        } else if (path.startsWith(QStringLiteral("/help"))) {
            QString app(QStringLiteral("kontact"));
            if (!url.query().isEmpty()) {
                app = url.query().mid(1);
            }
            KHelpClient::invokeHelp(QString(), app);
        }
    } else {
        new KRun(url, this);
    }
}

void MainWindow::readProperties(const KConfigGroup &config)
{
    Core::readProperties(config);

    QSet<QString> activePlugins =
        QSet<QString>::fromList(config.readEntry("ActivePlugins", QStringList()));

    if (!activePlugins.isEmpty()) {
        foreach (KontactInterface::Plugin *plugin, mPlugins) {
            if (!plugin->isRunningStandalone() && activePlugins.contains(plugin->identifier())) {
                plugin->readProperties(config);
            }
        }
    }
}

void MainWindow::saveProperties(KConfigGroup &config)
{
    Core::saveProperties(config);

    QStringList activePlugins;

    foreach (const KPluginInfo &pluginInfo, mPluginInfos) {
        if (pluginInfo.isPluginEnabled()) {
            KontactInterface::Plugin *plugin = pluginFromInfo(pluginInfo);
            if (plugin) {
                activePlugins.append(plugin->identifier());
                plugin->saveProperties(config);
            }
        }
    }

    config.writeEntry("ActivePlugins", activePlugins);
}

bool MainWindow::queryClose()
{
    if (qApp->isSavingSession() || mReallyClose) {
        return true;
    }

    foreach (KontactInterface::Plugin *plugin, mPlugins) {
        if (!plugin->isRunningStandalone()) {
            if (!plugin->queryClose()) {
                return false;
            }
        }
    }
    return true;
}

void MainWindow::slotShowStatusMsg(const QString &msg)
{
    if (!statusBar() || !mStatusMsgLabel) {
        return;
    }

    mStatusMsgLabel->setText(msg);
}

QVariantHash MainWindow::introductionData()
{
    QVariantHash data;
    data[QStringLiteral("icon")] = QStringLiteral("kontact");
    data[QStringLiteral("name")] = i18n("Kontact");
    data[QStringLiteral("subtitle")] = i18n("The KDE Personal Information Management Suite.");
    data[QStringLiteral("version")] = KAboutData::applicationData().version();

    QVariantList links = {
        QVariantHash{ { QStringLiteral("url"), QStringLiteral("exec:/help?kontact") },
                      { QStringLiteral("icon"), QStringLiteral("help-contents") },
                      { QStringLiteral("title"), i18n("Read Manual") },
                      { QStringLiteral("subtext"), i18n("Learn more about Kontact and its components") } },
        QVariantHash{ { QStringLiteral("url"), QStringLiteral("http://kontact.org") },
                      { QStringLiteral("icon"), QStringLiteral("kontact") },
                      { QStringLiteral("title"), i18n("Visit Kontact Website") },
                      { QStringLiteral("subtext"), i18n("Access online resources and tutorials") } },
        QVariantHash{ { QStringLiteral("url"), QStringLiteral("exec:/accountwizard") },
                      { QStringLiteral("icon"), QStringLiteral("tools-wizard") },
                      { QStringLiteral("title"), i18n("Setup your Accounts") },
                      { QStringLiteral("subtext"), i18n("Prepare Kontact for use") } }
    };
    data[QStringLiteral("links")] = links;

    return data;
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

void MainWindow::setHelpText(QAction *action, const QString &text)
{
    action->setStatusTip(text);
    action->setToolTip(text);
    if (action->whatsThis().isEmpty()) {
        action->setWhatsThis(text);
    }
}
