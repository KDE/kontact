/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef KONTACT_MAINWINDOW_H
#define KONTACT_MAINWINDOW_H

#include <qwidgetstack.h>
#include <qptrlist.h>

#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kdcopservicestarter.h>

#include "core.h"

class KAction;
class KPluginInfo;
class QHBox;
class QSplitter;
class QVBox;

namespace KParts
{
  class InfoExtension;
}

namespace Kontact
{

struct InfoExtData
{
  QString text;
  QPixmap pixmap;
};

class Plugin;
class SidePaneBase;
class AboutDialog;

class MainWindow : public Kontact::Core, public KDCOPServiceStarter
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

    virtual QValueList<Kontact::Plugin*> pluginList() const { return mPlugins; }

  public slots:
    virtual void selectPlugin( Kontact::Plugin *plugin );
    virtual void selectPlugin( const QString &pluginName );

    void updateConfig();

  signals:
    void textChanged( const QString& );
    void iconChanged( const QPixmap& );

  protected slots:
    void slotActivePartChanged( KParts::Part *part );
    void slotPreferences();
    void slotNewClicked();
    void slotQuit();
    void slotShowTip();
    void showAboutDialog();

  protected:
    void initWidgets();

    void loadSettings();
    void saveSettings();

    bool isPluginLoaded( const KPluginInfo * );
    void loadPlugins();
    void unloadPlugins();
    bool removePlugin( const KPluginInfo * );
    void addPlugin( Kontact::Plugin *plugin );
    void addPart( KParts::Part *part );
    void setupActions();
    void initHeaderWidget( QVBox *vBox );
    void showTip( bool );

  private slots:
    void pluginsChanged();
    void setHeaderText( const QString& );
    void setHeaderPixmap( const QPixmap& );

    void configureShortcuts();

  private:
    QWidget *mTopWidget;

    QHBox *mHeaderFrame;
    QLabel *mHeaderText;
    QLabel *mHeaderPixmap;
    QSplitter *mSplitter;

    KToolBarPopupAction *mNewActions;
    SidePaneBase *mSidePane;
    QWidgetStack *mStack;
    Plugin *mCurrentPlugin;
    KParts::PartManager *mPartManager;
    typedef QValueList<Kontact::Plugin*> PluginList;
    PluginList mPlugins;
    PluginList mDelayedPreload;
    QValueList<KPluginInfo*> mPluginInfos;
    KParts::InfoExtension *mLastInfoExtension;

    QMap<KParts::InfoExtension*, InfoExtData> mInfoExtCache;

    int mSidePaneType;
    //QStringList mActivePlugins;

    AboutDialog *mAboutDialog;
};

}

#endif
// vim: sw=2 sts=2 et
