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


#ifndef KONTACT_MAINWINDOW_H
#define KONTACT_MAINWINDOW_H

#include <qguardedptr.h>
#include <qptrlist.h>
#include <qwidgetstack.h>

#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kdcopservicestarter.h>

#include "core.h"
#include "kontactiface.h"

class QHBox;
class QSplitter;
class QVBox;
class QFrame;

class KAction;
class KConfig;
class KPluginInfo;
class KRSqueezedTextLabel;
class KHTMLPart;
class KeyPressEater;

namespace KPIM
{
  class StatusbarProgressWidget;
}

namespace Kontact
{

class Plugin;
class SidePaneBase;
class AboutDialog;

typedef QValueList<Kontact::Plugin*> PluginList;

class MainWindow : public Kontact::Core, public KDCOPServiceStarter, public KontactIface
{
  Q_OBJECT

  public:
    MainWindow();
    ~MainWindow();

    // KDCOPServiceStarter interface
    virtual int startServiceFor( const QString& serviceType,
                                 const QString& constraint = QString::null,
                                 const QString& preferences = QString::null,
                                 QString *error = 0, QCString* dcopService = 0,
                                 int flags = 0 );

    virtual PluginList pluginList() const { return mPlugins; }
    void setActivePluginModule( const QString & );

  public slots:
    virtual void selectPlugin( Kontact::Plugin *plugin );
    virtual void selectPlugin( const QString &pluginName );

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
    void slotRequestFeature();
    void slotConfigureProfiles();
    void slotLoadProfile( const QString& id );
    void slotSaveToProfile( const QString& id );
    void slotNewToolbarConfig();
    void slotShowIntroduction();
    void showAboutDialog();
    void slotShowStatusMsg( const QString& );
    void activatePluginModule();
    void slotOpenUrl( const KURL &url );

  private:
    void initWidgets();
    void initAboutScreen();
    void loadSettings();
    void saveSettings();

    bool isPluginLoaded( const KPluginInfo * );
    Kontact::Plugin *pluginFromInfo( const KPluginInfo * );
    void loadPlugins();
    void unloadPlugins();
    bool removePlugin( const KPluginInfo * );
    void addPlugin( Kontact::Plugin *plugin );
    void partLoaded( Kontact::Plugin *plugin, KParts::ReadOnlyPart *part );
    void setupActions();
    void showTip( bool );
    virtual bool queryClose();
    virtual void readProperties( KConfig *config );
    virtual void saveProperties( KConfig *config );
    void paintAboutScreen( const QString& msg );
    static QString introductionString();
    KToolBar* findToolBar(const char* name);

  private slots:
    void pluginsChanged();

    void configureShortcuts();
    void configureToolbars();

  private:
    QFrame *mTopWidget;

    QSplitter *mSplitter;

    KToolBarPopupAction *mNewActions;
    KToolBarPopupAction *mSyncActions;
    SidePaneBase *mSidePane;
    QWidgetStack *mPartsStack;
    Plugin *mCurrentPlugin;
    KParts::PartManager *mPartManager;
    PluginList mPlugins;
    PluginList mDelayedPreload;
    QValueList<KPluginInfo*> mPluginInfos;
    KHTMLPart *mIntroPart;

    KRSqueezedTextLabel* mStatusMsgLabel;
    KPIM::StatusbarProgressWidget *mLittleProgress;

    QString mActiveModule;

    QMap<QString, QGuardedPtr<QWidget> > mFocusWidgets;

    AboutDialog *mAboutDialog;
    bool mReallyClose;
    bool mSyncActionsEnabled;
};

}

#endif
// vim: sw=2 sts=2 et
