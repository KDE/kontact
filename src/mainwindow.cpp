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
#include "prefs.h"
#include "iconsidepane.h"

#include "webengine/introductionwebengineview.h"
#include "webengine/introductionwebenginepage.h"

#include "kontactconfiguredialog.h"
using namespace Kontact;
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
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
#include "kontact_debug.h"
#include <KEditToolBar>
#include <KHelpMenu>
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
#include <KLocalizedString>
#include <QDBusConnection>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QIcon>
#include <KHelpClient>
#include <KSharedConfig>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QApplication>
#include <KAboutData>

#include <grantleetheme/grantleethememanager.h>
#include <grantleetheme/grantleetheme.h>

// Define the maximum time Kontact waits for KSycoca to become available.
static const int KSYCOCA_WAIT_TIMEOUT = 10;

MainWindow::MainWindow()
    : KontactInterface::Core()
    , mSplitter(nullptr)
    , mCurrentPlugin(nullptr)
    , mAboutDialog(nullptr)
    , mReallyClose(false)
    , mSaveSideBarWidth(10)
{
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

    KStandardAction::keyBindings(this, &MainWindow::configureShortcuts, actionCollection());
    KStandardAction::configureToolbars(this, &MainWindow::configureToolbars, actionCollection());
    setXMLFile(QStringLiteral("kontactui.rc"));

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
        qCritical() << "Unable to find navigatorToolBar, probably kontactui.rc is missing";
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
    KService::List offers = KServiceTypeTrader::self()->query(
        QStringLiteral("Kontact/Plugin"),
        QStringLiteral("[X-KDE-KontactPluginVersion] == %1").arg(KONTACT_PLUGIN_VERSION));
    mPluginInfos = KPluginInfo::fromServices(
        offers, KConfigGroup(Prefs::self()->config(), "Plugins"));

    const KPluginInfo::List::Iterator end(mPluginInfos.end());
    for (KPluginInfo::List::Iterator it = mPluginInfos.begin(); it != end; ++it) {
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

    connect(KPIM::BroadcastStatus::instance(), &KPIM::BroadcastStatus::statusMsg, this, &MainWindow::slotShowStatusMsg);

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

    createGUI(nullptr);
    saveSettings();

    Prefs::self()->save();

    // During deletion of plugins, we should not access the plugin list (bug #182176)
    delete mSidePane;

    const PluginList::ConstIterator end = mPlugins.constEnd();
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

bool MainWindow::pluginWeightLessThan(const KontactInterface::Plugin *left, const KontactInterface::Plugin *right)
{
    return left->weight() < right->weight();
}

void MainWindow::activateInitialPluginModule()
{
    if (!mInitialActiveModule.isEmpty() && !mPlugins.isEmpty()) {
        const PluginList::ConstIterator end = mPlugins.constEnd();
        for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
            if (!(*it)->identifier().isEmpty()
                && (*it)->identifier().contains(mInitialActiveModule)) {
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
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    mTopWidget->setLayout(layout);
    setCentralWidget(mTopWidget);

    mSplitter = new QSplitter(mTopWidget);
    connect(mSplitter, &QSplitter::splitterMoved, this, &MainWindow::slotSplitterMoved);
    layout->addWidget(mSplitter);
    mSidePane = new IconSidePane(this, mSplitter);

    connect(mSidePane, SIGNAL(pluginSelected(KontactInterface::Plugin*)), SLOT(selectPlugin(KontactInterface::Plugin*)));

    mPartsStack = new QStackedWidget(mSplitter);
    mPartsStack->layout()->setSpacing(0);

    initAboutScreen();

    paintAboutScreen(QStringLiteral("loading_kontact.html"), QVariantHash());

    KPIM::ProgressStatusBarWidget *progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    mStatusMsgLabel
        = new KSqueezedTextLabel(i18nc("@info:status", " Initializing..."), statusBar());
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
                                        nullptr,
                                        QStringLiteral("messageviewer/about/"));
    GrantleeTheme::Theme theme = manager.theme(QStringLiteral("default"));
    if (!theme.isValid()) {
        qCDebug(KONTACT_LOG) << "Theme error: failed to find splash theme";
    } else {
        mIntroPart->setHtml(theme.render(templateName, data, QByteArrayLiteral("kontact")),
                            QUrl::fromLocalFile(theme.absolutePath() + QLatin1Char('/')));
    }
}

void MainWindow::initAboutScreen()
{
    QWidget *introbox = new QWidget(mPartsStack);
    QHBoxLayout *introboxHBoxLayout = new QHBoxLayout(introbox);
    introboxHBoxLayout->setContentsMargins(0, 0, 0, 0);
    mPartsStack->addWidget(introbox);
    mPartsStack->setCurrentWidget(introbox);
    mIntroPart = new IntroductionWebEngineView(introbox);
    connect(mIntroPart, &IntroductionWebEngineView::openUrl, this, &MainWindow::slotOpenUrl, Qt::QueuedConnection);
    introboxHBoxLayout->addWidget(mIntroPart);
}

void MainWindow::setupActions()
{
    actionCollection()->addAction(KStandardAction::Quit, this, SLOT(slotQuit()));

    mNewActions = new KActionMenu(
        i18nc("@title:menu create new pim items (message,calendar,to-do,etc.)", "New"), this);
    actionCollection()->addAction(QStringLiteral("action_new"), mNewActions);
    actionCollection()->setDefaultShortcuts(mNewActions, KStandardShortcut::openNew());
    connect(mNewActions, &KActionMenu::triggered, this, &MainWindow::slotNewClicked);

    QAction *action
        = new QAction(QIcon::fromTheme(QStringLiteral("configure")),
                      i18nc("@action:inmenu", "Configure Kontact..."), this);
    setHelpText(action, i18nc("@info:status", "Configure Kontact"));
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can configure Kontact."));
    actionCollection()->addAction(QStringLiteral("settings_configure_kontact"), action);
    connect(action, &QAction::triggered, this, &MainWindow::slotPreferences);

    action
        = new QAction(QIcon::fromTheme(QStringLiteral("kontact")),
                      i18nc("@action:inmenu", "&Kontact Introduction"), this);
    setHelpText(action, i18nc("@info:status", "Show the Kontact Introduction page"));
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to see the Kontact Introduction page."));
    actionCollection()->addAction(QStringLiteral("help_introduction"), action);
    connect(action, &QAction::triggered, this, &MainWindow::slotShowIntroduction);

    mShowHideAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-width")),
                                  i18nc("@action:inmenu", "Hide/Show the component sidebar"), this);
    setHelpText(mShowHideAction, i18nc("@info:status", "Hide/Show the component sidebar"));
    mShowHideAction->setCheckable(true);
    mShowHideAction->setChecked(Prefs::self()->sideBarOpen());
    mShowHideAction->setWhatsThis(
        i18nc("@info:whatsthis",
              "Allows you to show or hide the component sidebar as desired."));
    actionCollection()->addAction(QStringLiteral("hide_show_sidebar"), mShowHideAction);
    actionCollection()->setDefaultShortcut(mShowHideAction, QKeySequence(Qt::Key_F9));
    connect(mShowHideAction, &QAction::triggered, this, &MainWindow::slotShowHideSideBar);
}

bool MainWindow::isPluginLoaded(const KPluginInfo &info)
{
    return pluginFromInfo(info) != nullptr;
}

KontactInterface::Plugin *MainWindow::pluginFromInfo(const KPluginInfo &info)
{
    const PluginList::ConstIterator end = mPlugins.constEnd();
    for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
        if ((*it)->identifier() == info.pluginName()) {
            return *it;
        }
    }
    return nullptr;
}

void MainWindow::loadPlugins()
{
    QList<KontactInterface::Plugin *> plugins;

    int i;
    KPluginInfo::List::ConstIterator it;
    const KPluginInfo::List::ConstIterator end(mPluginInfos.constEnd());
    for (it = mPluginInfos.constBegin(); it != end; ++it) {
        if (!it->isPluginEnabled()) {
            continue;
        }

        KontactInterface::Plugin *plugin = nullptr;
        if (isPluginLoaded(*it)) {
            plugin = pluginFromInfo(*it);
            if (plugin) {
                plugin->configUpdated();
            }
            continue;
        }

        qCDebug(KONTACT_LOG) << "Loading Plugin:" << it->name();
        QString error;
        plugin
            = it->service()->createInstance<KontactInterface::Plugin>(this, QVariantList(), &error);

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
        const int nbPlugins{
            plugins.count()
        };
        for (i = 0; i < nbPlugins; ++i) {
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
        const QList<QAction *>::const_iterator end(actionList.end());

        for (QList<QAction *>::const_iterator listIt = actionList.begin(); listIt != end; ++listIt) {
            qCDebug(KONTACT_LOG) << QStringLiteral("Plugging New actions") << (*listIt)->objectName();
            mNewActions->addAction((*listIt));
        }

        addPlugin(plugin);
    }

    const bool state = (!mPlugins.isEmpty());
    mNewActions->setEnabled(state);
}

void MainWindow::unloadPlugins()
{
    const KPluginInfo::List::ConstIterator end = mPluginInfos.constEnd();
    for (KPluginInfo::List::ConstIterator it = mPluginInfos.constBegin(); it != end; ++it) {
        if (!it->isPluginEnabled()) {
            removePlugin(*it);
        }
    }
}

void MainWindow::updateShortcuts()
{
    const ActionPluginList::ConstIterator end = mActionPlugins.constEnd();
    int i = 0;
    for (ActionPluginList::ConstIterator it = mActionPlugins.constBegin(); it != end; ++it) {
        QAction *action = static_cast<QAction *>(*it);
        const QString shortcut = QStringLiteral("Ctrl+%1").arg(mActionPlugins.count() - i);
        actionCollection()->setDefaultShortcut(action, QKeySequence(shortcut));
        ++i;
    }
    factory()->plugActionList(this, QStringLiteral("navigator_actionlist"), mActionPlugins);
}

bool MainWindow::removePlugin(const KPluginInfo &info)
{
    const PluginList::Iterator end = mPlugins.end();
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
                delete q;
            }

            if (mCurrentPlugin == nullptr) {
                PluginList::Iterator it;
                const PluginList::Iterator pluginEnd(mPlugins.end());
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
    std::sort(mActionPlugins.begin(), mActionPlugins.end(), pluginActionWeightLessThan);
    std::sort(mPlugins.begin(), mPlugins.end(), pluginWeightLessThan);
    int i = 0;
    for (QAction *qaction : qAsConst(mActionPlugins)) {
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
        createGUI(nullptr);
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
        const PluginList::Iterator end(mPlugins.end());
        for (it = mPlugins.begin(); it != end; ++it) {
            if (!(*it)->newActions().isEmpty()) {
                (*it)->newActions().first()->trigger();
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

        QAction *newAction = nullptr;
        if (!plugin->newActions().isEmpty()) {
            newAction = plugin->newActions().at(0);
        }

        createGUI(plugin->part());
        plugin->shortcutChanged();

        // KParts's guiActivateEvent emptied the caption when leaving the old part,
        // and only some parts (like kmail) set it in guiActivateEvent when being activated.
        // For others, this is the default caption:
        if (windowTitle().isEmpty()) {
            setCaption(i18nc("@title:window Plugin dependent window title",
                             "%1 - Kontact", plugin->title()));
        }

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
    }

    KToolBar *navigatorToolBar = findToolBar("navigatorToolBar");
    if (navigatorToolBar && !navigatorToolBar->isHidden()
        && (toolBarArea(navigatorToolBar) == Qt::TopToolBarArea
            || toolBarArea(navigatorToolBar) == Qt::BottomToolBarArea)) {
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
        showHideSideBar(Prefs::self()->sideBarOpen());
    }

    // Preload Plugins. This _must_ happen before the default part is loaded
    PluginList::ConstIterator end(mDelayedPreload.constEnd());
    for (PluginList::ConstIterator it = mDelayedPreload.constBegin(); it != end; ++it) {
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
    static Kontact::KontactConfigureDialog *dlg = nullptr;
    if (!dlg) {
        dlg = new Kontact::KontactConfigureDialog(this);
        dlg->setAllowComponentSelection(true);

        // do not show settings of components running standalone
        KPluginInfo::List filteredPlugins = mPluginInfos;
        PluginList::ConstIterator end(mPlugins.constEnd());
        for (PluginList::ConstIterator it = mPlugins.constBegin(); it != end; ++it) {
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
        } else if (path.startsWith(QLatin1String("/help"))) {
            QString app(QStringLiteral("org.kde.kontact"));
            if (!url.query().isEmpty()) {
                app = url.query();
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

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QSet<QString> activePlugins
        = QSet<QString>::fromList(config.readEntry("ActivePlugins", QStringList()));
#else
    const auto activePluginList = config.readEntry("ActivePlugins", QStringList());
    QSet<QString> activePlugins(activePluginList.begin(), activePluginList.end());
#endif

    if (!activePlugins.isEmpty()) {
        for (KontactInterface::Plugin *plugin : qAsConst(mPlugins)) {
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

    for (const KPluginInfo &pluginInfo : qAsConst(mPluginInfos)) {
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

    for (KontactInterface::Plugin *plugin : qAsConst(mPlugins)) {
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
        QVariantHash{ { QStringLiteral("url"), QStringLiteral("exec:/help?org.kde.kontact") },
            { QStringLiteral("icon"), QStringLiteral("help-contents") },
            { QStringLiteral("title"), i18n("Read Manual") },
            { QStringLiteral("subtext"), i18n("Learn more about Kontact and its components") } },
        QVariantHash{ { QStringLiteral("url"), QStringLiteral("https://kontact.kde.org") },
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
    }
}

QString MainWindow::showHideSideBarMessage(bool hidden) const
{
    if (hidden) {
        return i18nc("@info:status",
                     "Sidebar is hidden. Show the sidebar again using the %1 key.",
                     mShowHideAction->shortcut().toString());
    } else {
        return QString();
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
