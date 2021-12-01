/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2005 Daniel Molkentin <molkentin@kde.org>
  SPDX-FileCopyrightText: 2003-2005 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kontact_export.h"
#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

class KActionMenu;
class KSqueezedTextLabel;
class QAction;
class QFrame;
class QSplitter;
class QStackedWidget;
class IntroductionWebEngineView;

using PluginList = QList<KontactInterface::Plugin *>;
using ActionPluginList = QList<QAction *>;

namespace Kontact
{
class AboutDialog;
class SidePaneBase;

class KONTACT_EXPORT MainWindow : public KontactInterface::Core
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kontact.KontactInterface")

public:
    MainWindow();
    ~MainWindow() override;

    PluginList pluginList() const override
    {
        return mPlugins;
    }

    void setInitialActivePluginModule(const QString &);

    static bool pluginActionWeightLessThan(const QAction *left, const QAction *right);
    static bool pluginWeightLessThan(const KontactInterface::Plugin *left, const KontactInterface::Plugin *right);
    void showHideSideBar(bool show);

public Q_SLOTS:
    void selectPlugin(KontactInterface::Plugin *plugin) override;
    Q_SCRIPTABLE void selectPlugin(const QString &pluginName) override;
    void slotActionTriggered(QAction *action, const QString &identifier);

    void updateConfig();

protected Q_SLOTS:
    void initObject();
    void initGUI();
    void slotActivePartChanged(KParts::Part *part);
    void slotPreferences();
    void slotNewClicked();
    void slotQuit();
    void slotNewToolbarConfig();
    void slotShowIntroduction();
    void showAboutDialog();
    void slotShowStatusMsg(const QString &);
    void activateInitialPluginModule();
    void slotOpenUrl(const QUrl &url);

private:
    void initWidgets();
    void initAboutScreen();
    void loadSettings();
    void saveSettings();

    KontactInterface::Plugin *pluginFromName(const QString &identifier) const;
    void loadPlugins();
    void unloadDisabledPlugins();
    void updateShortcuts();
    bool removePlugin(const QString &pluginName);
    void addPlugin(KontactInterface::Plugin *plugin);
    void partLoaded(KontactInterface::Plugin *plugin, KParts::Part *part) override;
    void setupActions();
    bool queryClose() override;
    void readProperties(const KConfigGroup &config) override;
    void saveProperties(KConfigGroup &config) override;
    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    static QVariantHash introductionData();
    KToolBar *findToolBar(const char *name);
    QString showHideSideBarMessage(bool hidden) const;

private Q_SLOTS:
    void pluginsChanged();

    void configureShortcuts();
    void configureToolbars() override;
    void slotShowHideSideBar();
    void slotSplitterMoved(int pos, int index);

private:
    void setHelpText(QAction *action, const QString &text);
    QFrame *mTopWidget = nullptr;

    QSplitter *mSplitter = nullptr;

    KActionMenu *mNewActions = nullptr;
    SidePaneBase *mSidePane = nullptr;
    QStackedWidget *mPartsStack = nullptr;
    KontactInterface::Plugin *mCurrentPlugin = nullptr;
    KParts::PartManager *mPartManager = nullptr;
    PluginList mPlugins;
    PluginList mDelayedPreload;
    ActionPluginList mActionPlugins;
    QVector<KPluginMetaData> mPluginMetaData;

    IntroductionWebEngineView *mIntroPart = nullptr;
    KSqueezedTextLabel *mStatusMsgLabel = nullptr;

    QString mInitialActiveModule;

    QMap<QString, QPointer<QWidget>> mFocusWidgets;
    QMap<KontactInterface::Plugin *, QAction *> mPluginAction;

    AboutDialog *mAboutDialog = nullptr;
    bool mReallyClose = false;
    int mSaveSideBarWidth = 10;
    QAction *mShowHideAction = nullptr;
};
}

Q_DECLARE_METATYPE(KontactInterface::Plugin *)

