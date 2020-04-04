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

#ifndef KONTACT_MAINWINDOW_H
#define KONTACT_MAINWINDOW_H

#include "kontact_export.h"
#include <KontactInterface/Core>
#include <KontactInterface/Plugin>
#include <QAction>

class KActionMenu;
class KPluginInfo;
class KSqueezedTextLabel;

class QFrame;
class QSplitter;
class QStackedWidget;
class IntroductionWebEngineView;

typedef QList<KontactInterface::Plugin *> PluginList;
typedef QList<QAction *> ActionPluginList;

namespace Kontact {
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
    void slotActionTriggered();

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

    bool isPluginLoaded(const KPluginInfo &);
    KontactInterface::Plugin *pluginFromInfo(const KPluginInfo &);
    void loadPlugins();
    void unloadPlugins();
    void updateShortcuts();
    bool removePlugin(const KPluginInfo &);
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
    QList<KPluginInfo> mPluginInfos;

    IntroductionWebEngineView *mIntroPart = nullptr;
    KSqueezedTextLabel *mStatusMsgLabel = nullptr;

    QString mInitialActiveModule;

    QMap<QString, QPointer<QWidget> > mFocusWidgets;
    QMap<KontactInterface::Plugin *, QAction *> mPluginAction;

    AboutDialog *mAboutDialog = nullptr;
    bool mReallyClose = false;
    int mSaveSideBarWidth = 0;
    QAction *mShowHideAction = nullptr;
};
}

Q_DECLARE_METATYPE(KontactInterface::Plugin *)

#endif
