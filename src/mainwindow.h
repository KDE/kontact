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
#include <kontactinterfaces/core.h>

#include <kparts/part.h>
#include <kparts/partmanager.h>

#include <QPointer>

class QFrame;
class QSplitter;
class QStackedWidget;

class KConfig;
class KPluginInfo;
class KSqueezedTextLabel;
class KHTMLPart;
class KActionMenu;

namespace KPIM {
  class StatusbarProgressWidget;
}

namespace Kontact {

class Plugin;
class SidePaneBase;
class AboutDialog;

typedef QList<Kontact::Plugin*> PluginList;
typedef QList<QAction*> ActionPluginList;

class KONTACT_EXPORT MainWindow : public Kontact::Core
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kde.kontact.KontactInterface" )

  public:
    MainWindow();
    ~MainWindow();

    virtual PluginList pluginList() const { return mPlugins; }
    void setActivePluginModule( const QString & );

    static bool pluginActionWeightLessThan( const QAction *left, const QAction *right );
    static bool pluginWeightLessThan( const Kontact::Plugin *left, const Kontact::Plugin *right );

  public slots:
    virtual void selectPlugin( Kontact::Plugin *plugin );
    Q_SCRIPTABLE virtual void selectPlugin( const QString &pluginName );
    void slotActionTriggered();

    void updateConfig();

  protected slots:
    void initObject();
    void initGUI();
    void slotActivePartChanged( KParts::Part *part );
    void slotPreferences();
    void slotNewClicked();
    void slotSyncClicked();
    void slotQuit();
    void slotShowTip();
    void slotConfigureProfiles();
    void slotLoadProfile( const QString& id );
    void slotSaveToProfile( const QString& id );
    void slotShowTipOnStart();
    void slotNewToolbarConfig();
    void slotShowIntroduction();
    void showAboutDialog();
    void slotShowStatusMsg( const QString & );
    void activatePluginModule();
    void slotOpenUrl( const KUrl &url );

  private:
    void initWidgets();
    void initAboutScreen();
    void loadSettings();
    void saveSettings();

    bool isPluginLoaded( const KPluginInfo & );
    Kontact::Plugin *pluginFromInfo( const KPluginInfo & );
    void loadPlugins();
    void unloadPlugins();
    void updateShortcuts();
    bool removePlugin( const KPluginInfo & );
    void addPlugin( Kontact::Plugin *plugin );
    void partLoaded( Kontact::Plugin *plugin, KParts::ReadOnlyPart *part );
    void setupActions();
    void showTip( bool );
    virtual bool queryClose();
    virtual void readProperties( const KConfigGroup &config );
    virtual void saveProperties( KConfigGroup &config );
    void paintAboutScreen( const QString &msg );
    static QString introductionString();
    KToolBar *findToolBar( const char *name );

  private slots:
    void pluginsChanged();

    void configureShortcuts();
    void configureToolbars();
    void slotSaveIconUsageLog();

  private:
    QFrame *mTopWidget;

    QSplitter *mSplitter;

    KActionMenu *mNewActions;
    KActionMenu *mSyncActions;
    SidePaneBase *mSidePane;
    QStackedWidget *mPartsStack;
    Plugin *mCurrentPlugin;
    KParts::PartManager *mPartManager;
    PluginList mPlugins;
    PluginList mDelayedPreload;
    ActionPluginList mActionPlugins;
    QList<KPluginInfo> mPluginInfos;
    KHTMLPart *mIntroPart;

    KSqueezedTextLabel *mStatusMsgLabel;
    KPIM::StatusbarProgressWidget *mLittleProgress;

    QString mActiveModule;

    QMap<QString, QPointer<QWidget> > mFocusWidgets;

    AboutDialog *mAboutDialog;
    bool mReallyClose;
    bool mSyncActionsEnabled;
};

}

Q_DECLARE_METATYPE( Kontact::Plugin * )

#endif
// vim: sw=2 sts=2 et
