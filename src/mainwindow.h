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
#include "config-kdepim.h"
#include <KontactInterface/Core>
#include <KontactInterface/Plugin>
#include <QAction>

class KActionMenu;
class KWebView;
class KPluginInfo;
class KSqueezedTextLabel;

class QFrame;
class QSplitter;
class QStackedWidget;
#ifdef QTWEBENGINE_EXPERIMENTAL_OPTION
class IntroductionWebEngineView;
#else
class IntroductionWebView;
#endif

typedef QList<KontactInterface::Plugin *> PluginList;
typedef QList<QAction *> ActionPluginList;

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
    ~MainWindow();

    PluginList pluginList() const Q_DECL_OVERRIDE
    {
        return mPlugins;
    }
    void setInitialActivePluginModule(const QString &);

    static bool pluginActionWeightLessThan(const QAction *left, const QAction *right);
    static bool pluginWeightLessThan(const KontactInterface::Plugin *left,
                                     const KontactInterface::Plugin *right);

public Q_SLOTS:
    void selectPlugin(KontactInterface::Plugin *plugin) Q_DECL_OVERRIDE;
    Q_SCRIPTABLE void selectPlugin(const QString &pluginName) Q_DECL_OVERRIDE;
    void slotActionTriggered();

    void updateConfig();

protected Q_SLOTS:
    void initObject();
    void initGUI();
    void slotActivePartChanged(KParts::Part *part);
    void slotPreferences();
    void slotNewClicked();
    void slotSyncClicked();
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
    void waitForKSycoca();

    bool isPluginLoaded(const KPluginInfo &);
    KontactInterface::Plugin *pluginFromInfo(const KPluginInfo &);
    void loadPlugins();
    void unloadPlugins();
    void updateShortcuts();
    bool removePlugin(const KPluginInfo &);
    void addPlugin(KontactInterface::Plugin *plugin);
    void partLoaded(KontactInterface::Plugin *plugin, KParts::ReadOnlyPart *part) Q_DECL_OVERRIDE;
    void setupActions();
    bool queryClose() Q_DECL_OVERRIDE;
    void readProperties(const KConfigGroup &config) Q_DECL_OVERRIDE;
    void saveProperties(KConfigGroup &config) Q_DECL_OVERRIDE;
    void paintAboutScreen(const QString &templateName, const QVariantHash &data);
    static QVariantHash introductionData();
    KToolBar *findToolBar(const char *name);

private Q_SLOTS:
    void pluginsChanged();

    void configureShortcuts();
    void configureToolbars() Q_DECL_OVERRIDE;
    void slotShowHideSideBar();

private:
    void setHelpText(QAction *action, const QString &text);
    QFrame *mTopWidget;

    QSplitter *mSplitter;

    KActionMenu *mNewActions;
    KActionMenu *mSyncActions;
    SidePaneBase *mSidePane;
    QStackedWidget *mPartsStack;
    KontactInterface::Plugin *mCurrentPlugin;
    KParts::PartManager *mPartManager;
    PluginList mPlugins;
    PluginList mDelayedPreload;
    ActionPluginList mActionPlugins;
    QList<KPluginInfo> mPluginInfos;

#ifdef QTWEBENGINE_EXPERIMENTAL_OPTION
    IntroductionWebEngineView *mIntroPart;
#else
    IntroductionWebView *mIntroPart;
#endif
    KSqueezedTextLabel *mStatusMsgLabel;

    QString mInitialActiveModule;

    QMap<QString, QPointer<QWidget> > mFocusWidgets;
    QMap<KontactInterface::Plugin *, QAction *> mPluginAction;

    AboutDialog *mAboutDialog;
    bool mReallyClose;
    bool mSyncActionsEnabled;
};

}

Q_DECLARE_METATYPE(KontactInterface::Plugin *)

#endif
