/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2005 Daniel Molkentin <molkentin@kde.org>
  SPDX-FileCopyrightText: 2003-2005 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
using namespace Qt::Literals::StringLiterals;

#include "config-kontact.h"
#include "iconsidepane.h"
#include "kontactconfiguredialog.h"
#include "prefs.h"
#include "webengine/introductionwebenginepage.h"
#include "webengine/introductionwebengineview.h"
using namespace Kontact;
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <Libkdepim/ProgressStatusBarWidget>
#include <Libkdepim/StatusbarProgressWidget>
#include <PimCommon/BroadcastStatus>

#include "kontact_debug.h"
#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KConfigGroup>
#include <KDialogJobUiDelegate>
#include <KEditToolBar>
#include <KHelpClient>
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/PartManager>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KSycoca>
#include <KToggleAction>
#include <KToolBar>
#include <KWindowConfig>
#include <KXMLGUIFactory>
#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QIcon>
#include <QMenuBar>
#include <QPointer>
#include <QShortcut>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWebEngineUrlScheme>

#include <GrantleeTheme/GrantleeTheme>
#include <GrantleeTheme/GrantleeThemeManager>

#include <PimCommon/NeedUpdateVersionUtils>
#include <PimCommon/NeedUpdateVersionWidget>
#include <PimCommon/VerifyNewVersionWidget>

// signal handler for SIGINT & SIGTERM
#ifdef Q_OS_UNIX
#include <KSignalHandler>
#include <csignal>
#include <unistd.h>
#endif

MainWindow::MainWindow()
    : KontactInterface::Core()
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    , mVerifyNewVersionWidget(new PimCommon::VerifyNewVersionWidget(this))
#endif
{
#ifdef Q_OS_UNIX
    /**
     * Set up signal handler for SIGINT and SIGTERM
     */
    KSignalHandler::self()->watchSignal(SIGINT);
    KSignalHandler::self()->watchSignal(SIGTERM);
    connect(KSignalHandler::self(), &KSignalHandler::signalReceived, this, [this](int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            // Intercept console.
            printf("Shutting down...\n");
            slotQuit();
        }
    });
#endif
    // Necessary for "cid" support in kmail.
    QWebEngineUrlScheme cidScheme("cid");
    cidScheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::ContentSecurityPolicyIgnored | QWebEngineUrlScheme::LocalScheme
                       | QWebEngineUrlScheme::LocalAccessAllowed);

    cidScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(cidScheme);

    QDBusConnection::sessionBus().registerObject(u"/KontactInterface"_s, this, QDBusConnection::ExportScriptableSlots);

    initGUI();
    initObject();

    mSidePane->setMaximumWidth(mSidePane->sizeHint().width());
    mSidePane->setMinimumWidth(mSidePane->sizeHint().width());

    factory()->plugActionList(this, u"navigator_actionlist"_s, mActionPlugins);

    KConfigGroup grp(KSharedConfig::openConfig(), u"MainWindow"_s);
    KWindowConfig::restoreWindowSize(windowHandle(), grp);
    setAutoSaveSettings();
    mShowMenuBarAction->setChecked(Prefs::self()->showMenuBar());
    slotToggleMenubar(true);
}

void MainWindow::initGUI()
{
    initWidgets();
    setupActions();

    KStandardActions::keyBindings(this, &MainWindow::configureShortcuts, actionCollection());
    KStandardActions::configureToolbars(this, &MainWindow::configureToolbars, actionCollection());
    setXMLFile(u"kontactui.rc"_s);

    setStandardToolBarMenuEnabled(true);

    createGUI(nullptr);

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
        qCCritical(KONTACT_LOG) << "Unable to find navigatorToolBar, probably kontactui.rc is missing";
    }
}

void MainWindow::initObject()
{
    if (!KSycoca::isAvailable()) {
        qDebug() << "Trying to create ksycoca...";
        KSycoca::self()->ensureCacheValid();
        if (!KSycoca::isAvailable()) {
            // This should only happen if the distribution is broken, or the disk full
            qFatal("KSycoca unavailable. Kontact will be unable to find plugins.");
        }
    }
    mPluginMetaData = KPluginMetaData::findPlugins(u"pim6/kontact"_s, [](const KPluginMetaData &data) {
        return data.rawData().value(u"X-KDE-KontactPluginVersion"_s).toInt() == KONTACT_PLUGIN_VERSION;
    });

    // prepare the part manager
    mPartManager = new KParts::PartManager(this);
    connect(mPartManager, &KParts::PartManager::activePartChanged, this, &MainWindow::slotActivePartChanged);

    loadPlugins();

    if (mSidePane) {
        mSidePane->updatePlugins();
    }

    loadSettings();

    statusBar()->show();

    // done initializing
    slotShowStatusMsg(QString());

    connect(PimCommon::BroadcastStatus::instance(), &PimCommon::BroadcastStatus::statusMsg, this, &MainWindow::slotShowStatusMsg);

    // launch commandline specified module if any
    activateInitialPluginModule();

    if (Prefs::lastVersionSeen() == KAboutData::applicationData().version()) {
        selectPlugin(mCurrentPlugin);
    }

    paintAboutScreen(u"introduction_kontact.html"_s, introductionData());
    Prefs::setLastVersionSeen(KAboutData::applicationData().version());
}

MainWindow::~MainWindow()
{
    if (mCurrentPlugin) {
        KConfigGroup grp(KSharedConfig::openConfig()->group(u"MainWindow%1"_s.arg(mCurrentPlugin->identifier())));
        saveMainWindowSettings(grp);
    }

    createGUI(nullptr);
    saveSettings();

    Prefs::self()->save();

    // During deletion of plugins, we should not access the plugin list (bug #182176)
    delete mSidePane;

    qDeleteAll(mPlugins);
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
    return !pluginWeightLessThan(left->data().value<KontactInterface::Plugin *>(), right->data().value<KontactInterface::Plugin *>());
}

bool MainWindow::pluginWeightLessThan(const KontactInterface::Plugin *left, const KontactInterface::Plugin *right)
{
    return left->weight() < right->weight();
}

void MainWindow::activateInitialPluginModule()
{
    if (!mInitialActiveModule.isEmpty() && !mPlugins.isEmpty()) {
        for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
            if (!plugin->identifier().isEmpty() && plugin->identifier().contains(mInitialActiveModule)) {
                selectPlugin(plugin);
                return;
            }
        }
    }
}

void MainWindow::initWidgets()
{
    auto mTopWidget = new QWidget(this);
    auto layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    mTopWidget->setLayout(layout);
    setCentralWidget(mTopWidget);

    if (PimCommon::NeedUpdateVersionUtils::checkVersion()) {
        const auto status = PimCommon::NeedUpdateVersionUtils::obsoleteVersionStatus(QLatin1String(KONTACT_RELEASE_VERSION_DATE), QDate::currentDate());
        if (status != PimCommon::NeedUpdateVersionUtils::ObsoleteVersion::NotObsoleteYet) {
            auto needUpdateVersionWidget = new PimCommon::NeedUpdateVersionWidget(this);
            layout->addWidget(needUpdateVersionWidget);
            needUpdateVersionWidget->setObsoleteVersion(status);
        }
    }

    mSplitter = new QSplitter(mTopWidget);
    connect(mSplitter, &QSplitter::splitterMoved, this, &MainWindow::slotSplitterMoved);
    layout->addWidget(mSplitter);
    mSidePane = new IconSidePane(this, mSplitter);

    connect(mSidePane, SIGNAL(pluginSelected(KontactInterface::Plugin *)), SLOT(selectPlugin(KontactInterface::Plugin *)));

    mPartsStack = new QStackedWidget(mSplitter);
    mPartsStack->layout()->setSpacing(0);

    initAboutScreen();

    paintAboutScreen(u"loading_kontact.html"_s, QVariantHash());

    auto progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    mStatusMsgLabel = new KSqueezedTextLabel(i18nc("@info:status", " Initializing..."), statusBar());
    mStatusMsgLabel->setTextElideMode(Qt::ElideRight);
    mStatusMsgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    statusBar()->addWidget(mStatusMsgLabel, 10);
    statusBar()->addPermanentWidget(progressStatusBarWidget->littleProgress(), 0);

    mSplitter->setCollapsible(1, false);
}

void MainWindow::paintAboutScreen(const QString &templateName, const QVariantHash &data)
{
    GrantleeTheme::ThemeManager manager(u"splashPage"_s, u"splash.theme"_s, nullptr, QStringLiteral("messageviewer/about/"));
    GrantleeTheme::Theme theme = manager.theme(u"default"_s);
    if (!theme.isValid()) {
        qCDebug(KONTACT_LOG) << "Theme error: failed to find splash theme";
    } else {
        mIntroPart->setHtml(theme.render(templateName, data, QByteArrayLiteral("kontact")), QUrl::fromLocalFile(theme.absolutePath() + u'/'));
    }
}

void MainWindow::initAboutScreen()
{
    auto introbox = new QWidget(mPartsStack);
    auto introboxHBoxLayout = new QHBoxLayout(introbox);
    introboxHBoxLayout->setContentsMargins({});
    mPartsStack->addWidget(introbox);
    mPartsStack->setCurrentWidget(introbox);
    mIntroPart = new IntroductionWebEngineView(introbox);
    connect(mIntroPart, &IntroductionWebEngineView::openUrl, this, &MainWindow::slotOpenUrl, Qt::QueuedConnection);
    introboxHBoxLayout->addWidget(mIntroPart);
}

void MainWindow::setupActions()
{
    KStandardActions::quit(this, &MainWindow::slotQuit, actionCollection());

    mNewActions = new KActionMenu(i18nc("@title:menu create new pim items (message,calendar,to-do,etc.)", "New"), this);
    actionCollection()->addAction(u"action_new"_s, mNewActions);
    actionCollection()->setDefaultShortcuts(mNewActions, KStandardShortcut::openNew());
    connect(mNewActions, &KActionMenu::triggered, this, &MainWindow::slotNewClicked);

    auto action = new QAction(QIcon::fromTheme(u"configure"_s), i18nc("@action:inmenu", "Configure Kontact..."), this);
    setHelpText(action, i18nc("@info:status", "Configure Kontact"));
    action->setWhatsThis(i18nc("@info:whatsthis", "You will be presented with a dialog where you can configure Kontact."));
    actionCollection()->addAction(u"settings_configure_kontact"_s, action);
    connect(action, &QAction::triggered, this, &MainWindow::slotPreferences);

    action = new QAction(QIcon::fromTheme(u"kontact"_s), i18nc("@action:inmenu", "&Kontact Introduction"), this);
    setHelpText(action, i18nc("@info:status", "Show the Kontact Introduction page"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Choose this option to see the Kontact Introduction page."));
    actionCollection()->addAction(u"help_introduction"_s, action);
    connect(action, &QAction::triggered, this, &MainWindow::slotShowIntroduction);

    mShowHideAction = new QAction(QIcon::fromTheme(u"zoom-fit-width"_s), i18nc("@action:inmenu", "Show Sidebar"), this);
    setHelpText(mShowHideAction, i18nc("@info:status", "Hide/Show the component sidebar"));
    mShowHideAction->setCheckable(true);
    mShowHideAction->setChecked(Prefs::self()->sideBarOpen());
    mShowHideAction->setWhatsThis(i18nc("@info:whatsthis", "Allows you to show or hide the component sidebar as desired."));
    actionCollection()->addAction(u"hide_show_sidebar"_s, mShowHideAction);
    actionCollection()->setDefaultShortcut(mShowHideAction, QKeySequence(Qt::Key_F9));
    connect(mShowHideAction, &QAction::triggered, this, &MainWindow::slotShowHideSideBar);

    mShowFullScreenAction = KStandardAction::fullScreen(nullptr, nullptr, this, actionCollection());
    actionCollection()->setDefaultShortcut(mShowFullScreenAction, Qt::Key_F11);
    connect(mShowFullScreenAction, &QAction::toggled, this, &MainWindow::slotFullScreen);

    auto manager = KColorSchemeManager::instance();
    actionCollection()->addAction(u"colorscheme_menu"_s, KColorSchemeMenu::createMenu(manager, this));

    mShowMenuBarAction = KStandardAction::showMenubar(this, &MainWindow::slotToggleMenubar, actionCollection());

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
#if KONTACT_STABLE_VERSION
    const QString url = u"https://cdn.kde.org/ci-builds/pim/kontact/25.08/windows/"_s;
#else
    const QString url = u"https://cdn.kde.org/ci-builds/pim/kontact/master/windows/"_s;
#endif
    mVerifyNewVersionWidget->addOsUrlInfo(PimCommon::VerifyNewVersionWidget::OsVersion::Windows, url);
    auto verifyNewVersionAction = mVerifyNewVersionWidget->verifyNewVersionAction();
    actionCollection()->addAction(u"verify_check_version"_s, verifyNewVersionAction);
#endif
}

void MainWindow::slotToggleMenubar(bool dontShowWarning)
{
    if (menuBar()) {
        if (mShowMenuBarAction->isChecked()) {
            menuBar()->show();
        } else {
            if (!dontShowWarning && (!toolBar()->isVisible() /* || !toolBar()->actions().contains(mHamburgerMenu)*/)) {
                const QString accel = mShowMenuBarAction->shortcut().toString(QKeySequence::NativeText);
                KMessageBox::information(this,
                                         i18n("<qt>This will hide the menu bar completely."
                                              " You can show it again by typing %1.</qt>",
                                              accel),
                                         i18nc("@title:window", "Hide menu bar"),
                                         u"HideMenuBarWarning"_s);
            }
            menuBar()->hide();
        }
        Prefs::self()->setShowMenuBar(mShowMenuBarAction->isChecked());
    }
}

void MainWindow::slotFullScreen(bool t)
{
    KToggleFullScreenAction::setFullScreen(this, t);
    QMenuBar *mb = menuBar();
    if (t) {
        auto b = new QToolButton(mb);
        b->setDefaultAction(mShowFullScreenAction);
        b->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored));
        b->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
        mb->setCornerWidget(b, Qt::TopRightCorner);
        b->setVisible(true);
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    } else {
        QWidget *w = mb->cornerWidget(Qt::TopRightCorner);
        if (w) {
            w->deleteLater();
        }
    }
}

KontactInterface::Plugin *MainWindow::pluginFromName(const QString &identifier) const
{
    auto hasIdentifier = [&](KontactInterface::Plugin *plugin) {
        return plugin->identifier() == identifier;
    };
    const auto it = std::find_if(mPlugins.constBegin(), mPlugins.constEnd(), hasIdentifier);
    return it == mPlugins.constEnd() ? nullptr : *it;
}

void MainWindow::loadPlugins()
{
    QList<KontactInterface::Plugin *> plugins;
    KConfigGroup configGroup(Prefs::self()->config(), u"Plugins"_s);
    for (const KPluginMetaData &pluginMetaData : std::as_const(mPluginMetaData)) {
        if (!configGroup.readEntry(pluginMetaData.pluginId() + "Enabled"_L1, pluginMetaData.isEnabledByDefault())) {
            continue;
        }

        KontactInterface::Plugin *plugin = pluginFromName(pluginMetaData.pluginId());
        if (plugin) { // already loaded
            plugin->configUpdated();
            continue;
        }

        qCDebug(KONTACT_LOG) << "Loading Plugin:" << pluginMetaData.name();
        const auto loadResult = KPluginFactory::instantiatePlugin<KontactInterface::Plugin>(pluginMetaData, this);
        if (!loadResult) {
            qCWarning(KONTACT_LOG) << "Error loading plugin" << pluginMetaData.fileName() << loadResult.errorString;
            continue;
        } else {
            plugin = loadResult.plugin;
            if (!plugin) {
                qCWarning(KONTACT_LOG) << "Unable to create plugin for" << pluginMetaData.fileName();
                continue;
            }
        }
        plugin->setIdentifier(pluginMetaData.pluginId());
        plugin->setTitle(pluginMetaData.name());
        plugin->setIcon(pluginMetaData.iconName());

        const QString libNameProp = pluginMetaData.value(u"X-KDE-KontactPartLibraryName"_s);
        const QString exeNameProp = pluginMetaData.value(u"X-KDE-KontactPartExecutableName"_s);

        if (pluginMetaData.rawData().contains("X-KDE-KontactPartLoadOnStart"_L1)) {
            const bool loadOnStart = pluginMetaData.rawData().value(u"X-KDE-KontactPartLoadOnStart"_s).toBool();
            if (loadOnStart) {
                mDelayedPreload.append(plugin);
            }
        }
        if (pluginMetaData.rawData().contains("X-KDE-KontactPluginHasPart"_L1)) {
            const bool hasPartProp = pluginMetaData.rawData().value(u"X-KDE-KontactPluginHasPart"_s).toBool();
            plugin->setShowInSideBar(hasPartProp);
        } else {
            plugin->setShowInSideBar(true);
        }

        qCDebug(KONTACT_LOG) << "LIBNAMEPART:" << libNameProp;

        plugin->setPartLibraryName(libNameProp.toUtf8());
        plugin->setExecutableName(exeNameProp);
        plugins.append(plugin);
    }
    std::sort(plugins.begin(), plugins.end(), pluginWeightLessThan); // new plugins

    for (KontactInterface::Plugin *plugin : std::as_const(plugins)) {
        const QList<QAction *> actionList = plugin->newActions();
        for (QAction *action : actionList) {
            qCDebug(KONTACT_LOG) << "Plugging new action" << action->objectName();
            mNewActions->addAction(action);
        }
        addPlugin(plugin);
    }

    std::sort(mPlugins.begin(), mPlugins.end(), pluginWeightLessThan); // all plugins

    // sort the action plugins again and reset shortcuts. If we removed and then read some plugins
    // we need to take in count their weights for setting shortcuts again
    std::sort(mActionPlugins.begin(), mActionPlugins.end(), pluginActionWeightLessThan);

    updateShortcuts();

    const bool state = (!mPlugins.isEmpty());
    mNewActions->setEnabled(state);
}

void MainWindow::unloadDisabledPlugins()
{
    // Only remove the now-disabled plugins.
    // Keep the other ones. loadPlugins() will skip those that are already loaded.
    KConfigGroup configGroup(Prefs::self()->config(), u"Plugins"_s);
    for (const KPluginMetaData &pluginMetaData : std::as_const(mPluginMetaData)) {
        if (!configGroup.readEntry(pluginMetaData.pluginId() + "Enabled"_L1, pluginMetaData.isEnabledByDefault())) {
            removePlugin(pluginMetaData.pluginId());
        }
    }
}

void MainWindow::updateShortcuts()
{
    for (int i = 0, total = mActionPlugins.count(); i < total; ++i) {
        QAction *action = mActionPlugins.at(i);
        const QKeySequence shortcut(u"Ctrl+%1"_s.arg(mActionPlugins.count() - i));
        actionCollection()->setDefaultShortcut(action, shortcut);
        // Prevent plugActionList from restoring some old saved shortcuts
        action->setProperty("_k_DefaultShortcut", QVariant::fromValue(QList<QKeySequence>{shortcut}));
    }
}

bool MainWindow::removePlugin(const QString &pluginName)
{
    auto hasIdentifier = [&](KontactInterface::Plugin *plugin) {
        return plugin->identifier() == pluginName;
    };
    const auto it = std::find_if(mPlugins.begin(), mPlugins.end(), hasIdentifier);
    if (it == mPlugins.end()) {
        qCDebug(KONTACT_LOG) << "Plugin not found" << pluginName;
        return false;
    }
    KontactInterface::Plugin *plugin = *it;
    const QList<QAction *> actionList = plugin->newActions();
    for (QAction *action : actionList) {
        qCDebug(KONTACT_LOG) << u"Unplugging New actions"_s << action->objectName();
        mNewActions->removeAction(action);
    }

    removeChildClient(plugin);

    if (mCurrentPlugin == plugin) {
        mCurrentPlugin = nullptr;
        createGUI(nullptr);
    }

    plugin->deleteLater(); // removes the part automatically
    mPlugins.erase(it);
    if (plugin->showInSideBar()) {
        QAction *q = mPluginAction[plugin]; // remove QAction, to free the shortcut for later use
        mActionPlugins.removeAll(q);
        mPluginAction.remove(plugin);
        actionCollection()->removeAction(q); // deletes q
    }

    if (mCurrentPlugin == nullptr) {
        for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
            if (plugin->showInSideBar()) {
                selectPlugin(plugin);
                return true;
            }
        }
    }
    return true;
}

void MainWindow::addPlugin(KontactInterface::Plugin *plugin)
{
    qCDebug(KONTACT_LOG);

    mPlugins.append(plugin);

    if (plugin->showInSideBar()) {
        auto action = new QAction(QIcon::fromTheme(plugin->icon()), plugin->title(), this);
        // action->setHelpText(
        //            i18nc( "@info:status", "Plugin %1", plugin->title() ) );
        action->setWhatsThis(i18nc("@info:whatsthis", "Switch to plugin %1", plugin->title()));
        action->setCheckable(true);
        action->setData(QVariant::fromValue(plugin)); // used by pluginActionWeightLessThan
        connect(action, &QAction::triggered, this, [this, action, plugin]() {
            slotActionTriggered(action, plugin->identifier());
        });
        actionCollection()->addAction(plugin->identifier(), action);
        mActionPlugins.append(action);
        mPluginAction.insert(plugin, action);
    }

    // merge the plugins GUI into the main window
    insertChildClient(plugin);
}

void MainWindow::partLoaded(KontactInterface::Plugin *plugin, KParts::Part *part)
{
    Q_UNUSED(plugin)

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
        createGUI(nullptr);
        return;
    }

    qCDebug(KONTACT_LOG) << u"Part activated:"_s << part << u"with stack id."_s << mPartsStack->indexOf(part->widget());

    statusBar()->clearMessage();
}

void MainWindow::slotNewClicked()
{
    if (!mCurrentPlugin || !mCurrentPlugin->newActions().isEmpty()) {
        mCurrentPlugin->newActions().at(0)->trigger();
    } else {
        for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
            if (!plugin->newActions().isEmpty()) {
                plugin->newActions().constFirst()->trigger();
                return;
            }
        }
    }
}

KToolBar *MainWindow::findToolBar(const char *name)
{
    // like KMainWindow::toolBar, but which doesn't create the toolbar if not found
    return findChild<KToolBar *>(QLatin1StringView(name));
}

void MainWindow::selectPlugin(KontactInterface::Plugin *plugin)
{
    if (!plugin) {
        return;
    }

    if (plugin->isRunningStandalone()) {
        statusBar()->showMessage(i18nc("@info:status", "Application is running standalone. Foregrounding..."), 1000);
        plugin->bringToForeground();
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (mCurrentPlugin) {
        KConfigGroup grp = KSharedConfig::openConfig()->group(u"MainWindow%1"_s.arg(mCurrentPlugin->identifier()));
        saveMainWindowSettings(grp);
    }

    KParts::Part *part = plugin->part();

    if (!part) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error(this, i18nc("@info", "Cannot load part for %1.", plugin->title()) + u'\n' + lastErrorMessage());
        plugin->setDisabled(true);
        mSidePane->updatePlugins();
        return;
    }

    if (mCurrentPlugin) {
        QAction *action = mPluginAction.value(mCurrentPlugin);
        if (action) {
            action->setChecked(false);
        }
    }
    QAction *selectedPluginAction = mPluginAction.value(mCurrentPlugin);
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
            focusWidget = mFocusWidgets.value(plugin->identifier());
            if (focusWidget) {
                focusWidget->setFocus();
            }
        } else {
            view->setFocus();
        }

        mCurrentPlugin = plugin;

        QAction *newAction = nullptr;
        if (!plugin->newActions().isEmpty()) {
            newAction = plugin->newActions().at(0);
        }

        const QString oldWindowTitle = windowTitle();

        createGUI(plugin->part());
        plugin->shortcutChanged();

        // Only some parts (like kmail) set a caption in guiActivateEvent when being activated.
        // For others, this is the default caption:
        if (windowTitle() == oldWindowTitle) {
            setCaption(i18nc("@title:window Plugin dependent window title", "%1 - Kontact", plugin->title()));
        }

        if (newAction) {
            mNewActions->setIcon(newAction->icon());
            mNewActions->setText(newAction->text());
            mNewActions->setWhatsThis(newAction->whatsThis());
        } else { // we'll use the action of the first plugin which offers one
            for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
                if (!plugin->newActions().isEmpty()) {
                    newAction = plugin->newActions().constFirst();
                }
                if (newAction) {
                    mNewActions->setIcon(newAction->icon());
                    mNewActions->setText(newAction->text());
                    mNewActions->setWhatsThis(newAction->whatsThis());
                    break;
                }
            }
        }
    }

    KToolBar *navigatorToolBar = findToolBar("navigatorToolBar");
    if (navigatorToolBar && !navigatorToolBar->isHidden()
        && (toolBarArea(navigatorToolBar) == Qt::TopToolBarArea || toolBarArea(navigatorToolBar) == Qt::BottomToolBarArea)) {
        addToolBar(toolBarArea(navigatorToolBar), navigatorToolBar);
    }

    applyMainWindowSettings(KSharedConfig::openConfig()->group(u"MainWindow%1"_s.arg(plugin->identifier())));

    QApplication::restoreOverrideCursor();
}

void MainWindow::slotActionTriggered(QAction *action, const QString &identifier)
{
    action->setChecked(true);
    if (!identifier.isEmpty()) {
        mSidePane->setCurrentPlugin(identifier);
    }
}

void MainWindow::selectPlugin(const QString &pluginName)
{
    KontactInterface::Plugin *plugin = pluginFromName(pluginName);
    if (plugin) {
        selectPlugin(plugin);
    }
}

void MainWindow::loadSettings()
{
    if (mSplitter) {
        showHideSideBar(Prefs::self()->sideBarOpen());
    }

    // Preload Plugins. This _must_ happen before the default part is loaded
    for (KontactInterface::Plugin *plugin : std::as_const(mDelayedPreload)) {
        selectPlugin(plugin);
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
    static Kontact::KontactConfigureDialog *dlg = nullptr;
    if (!dlg) {
        dlg = new Kontact::KontactConfigureDialog(this);
        connect(dlg, &KontactSettingsDialog::configCommitted, this, [this](const QString &pluginid) {
            if (pluginid == QLatin1String("kcm_kontact")) {
                MainWindow::updateConfig();
            }
        });

        // Add the main contact KCM which is not associated with a specific plugin
        dlg->addModule(KPluginMetaData(u"pim6/kcms/kontact/kcm_kontact"_s));

        auto sortByWeight = [](const KPluginMetaData &m1, const KPluginMetaData &m2) {
            return m1.rawData().value(u"X-KDE-Weight"_s).toInt() < m2.rawData().value(u"X-KDE-Weight"_s).toInt();
        };
        std::sort(mPluginMetaData.begin(), mPluginMetaData.end(), sortByWeight);
        for (const KPluginMetaData &metaData : std::as_const(mPluginMetaData)) {
            const QString pluginNamespace = metaData.value(u"X-KDE-ConfigModuleNamespace"_s);
            if (!pluginNamespace.isEmpty()) {
                auto plugins = KPluginMetaData::findPlugins(pluginNamespace);
                std::sort(plugins.begin(), plugins.end(), sortByWeight);
                dlg->addPluginComponent(metaData, plugins);
            }
        }
    }

    dlg->show();
}

// Called when the user enables/disables plugins in the configuration dialog
void MainWindow::pluginsChanged()
{
    unplugActionList(u"navigator_actionlist"_s);
    unloadDisabledPlugins();
    loadPlugins();
    mSidePane->updatePlugins();
    factory()->plugActionList(this, u"navigator_actionlist"_s, mActionPlugins);
}

void MainWindow::updateConfig()
{
    qCDebug(KONTACT_LOG);

    saveSettings();
    loadSettings();
}

void MainWindow::configureShortcuts()
{
    KShortcutsDialog dialog(this);
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
        KConfigGroup grp(KSharedConfig::openConfig()->group(u"MainWindow%1"_s.arg(mCurrentPlugin->identifier())));
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
        applyMainWindowSettings(KSharedConfig::openConfig()->group(u"MainWindow%1"_s.arg(mCurrentPlugin->identifier())));
    }
    factory()->plugActionList(this, u"navigator_actionlist"_s, mActionPlugins);
}

void MainWindow::slotOpenUrl(const QUrl &url)
{
    if (url.scheme() == "exec"_L1) {
        const QString path(url.path());
        if (path == "/switch"_L1) {
            if (mCurrentPlugin) {
                mPartsStack->setCurrentIndex(mPartsStack->indexOf(mCurrentPlugin->part()->widget()));
            }
        } else if (path == "/accountwizard"_L1) {
            auto job = new KIO::CommandLauncherJob(u"accountwizard"_s);
            job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
            job->exec();
            slotQuit();
        } else if (path.startsWith("/help"_L1)) {
            QString app(u"org.kde.kontact"_s);
            if (!url.query().isEmpty()) {
                app = url.query();
            }
            KHelpClient::invokeHelp(QString(), app);
        }
    } else {
        auto job = new KIO::OpenUrlJob(url);
        job->start();
    }
}

void MainWindow::readProperties(const KConfigGroup &config)
{
    Core::readProperties(config);

    const auto activePluginList = config.readEntry("ActivePlugins", QStringList());
    QSet<QString> activePlugins(activePluginList.begin(), activePluginList.end());

    if (!activePlugins.isEmpty()) {
        for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
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

    KConfigGroup configGroup(Prefs::self()->config(), u"Plugins"_s);
    for (const KPluginMetaData &pluginMetaData : std::as_const(mPluginMetaData)) {
        if (!configGroup.readEntry(pluginMetaData.pluginId() + "Enabled"_L1, pluginMetaData.isEnabledByDefault())) {
            KontactInterface::Plugin *plugin = pluginFromName(pluginMetaData.pluginId());
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

    for (KontactInterface::Plugin *plugin : std::as_const(mPlugins)) {
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
    data[u"icon"_s] = u"kontact"_s;
    data[u"name"_s] = i18n("Kontact");
    data[u"subtitle"_s] = i18n("The KDE Personal Information Management Suite.");
    data[u"version"_s] = KAboutData::applicationData().version();

    QVariantList links = {QVariantHash{{u"url"_s, u"exec:/help?org.kde.kontact"_s},
                                       {u"icon"_s, u"help-contents"_s},
                                       {u"title"_s, i18n("Read Manual")},
                                       {u"subtext"_s, i18n("Learn more about Kontact and its components")}},
                          QVariantHash{{u"url"_s, u"https://kontact.kde.org"_s},
                                       {u"icon"_s, u"kontact"_s},
                                       {u"title"_s, i18n("Visit Kontact Website")},
                                       {u"subtext"_s, i18n("Access online resources and tutorials")}},
                          QVariantHash{{u"url"_s, u"exec:/accountwizard"_s},
                                       {u"icon"_s, u"tools-wizard"_s},
                                       {u"title"_s, i18n("Setup your Accounts")},
                                       {u"subtext"_s, i18n("Prepare Kontact for use")}}};
    data[u"links"_s] = links;

    return data;
}

void MainWindow::showHideSideBar(bool show)
{
    QList<int> sizes = mSplitter->sizes();
    if (!sizes.isEmpty()) {
        if (show) {
            sizes[0] = mSaveSideBarWidth;
        } else {
            mSaveSideBarWidth = qMax(sizes[0], 10);
            sizes[0] = 0;
        }
        mSplitter->setSizes(sizes);
        Prefs::self()->setSideBarOpen(show);
        mShowHideAction->setChecked(show);
    }
}

QString MainWindow::showHideSideBarMessage(bool hidden) const
{
    if (hidden) {
        return i18nc("@info:status",
                     "Sidebar is hidden. Show the sidebar again using the %1 key.",
                     mShowHideAction->shortcut().toString(QKeySequence::NativeText));
    } else {
        return {};
    }
}

void MainWindow::slotShowHideSideBar()
{
    QList<int> sizes = mSplitter->sizes();
    if (!sizes.isEmpty()) {
        bool open = (sizes.at(0) != 0);
        showHideSideBar(!open);
        if (open) {
            statusBar()->showMessage(showHideSideBarMessage(true));
        } else {
            statusBar()->showMessage(showHideSideBarMessage(false));
        }
    }
}

void MainWindow::slotSplitterMoved(int pos, int index)
{
    if (index == 1) {
        if (pos == 0) {
            statusBar()->showMessage(showHideSideBarMessage(true));
        } else {
            statusBar()->showMessage(showHideSideBarMessage(false));
        }
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

#include "moc_mainwindow.cpp"
