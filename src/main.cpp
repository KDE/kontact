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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <iostream>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstartupinfo.h>
#include <kuniqueapplication.h>
#include <kwin.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include "plugin.h"

#include <qlabel.h>
#include "prefs.h"

#include "alarmclient.h"
#include "mainwindow.h"
#include <uniqueapphandler.h> // in ../interfaces
#include "profilemanager.h"

using namespace std;

static const char description[] =
    I18N_NOOP( "KDE personal information manager" );

static const char version[] = "1.2.9 (enterprise35 0.20090227.932370)";

class KontactApp : public KUniqueApplication {
  public:
    KontactApp() : mMainWindow( 0 ), mSessionRestored( false ) {}
    ~KontactApp() {}

    int newInstance();
    void setMainWindow( Kontact::MainWindow *window ) {
        mMainWindow = window;
        setMainWidget( window );
    }
    void setSessionRestored( bool restored ) {
        mSessionRestored = restored;
    }

  private:
    void startKOrgac();
    Kontact::MainWindow *mMainWindow;
    bool mSessionRestored;
};

static void listPlugins()
{
  KInstance instance( "kontact" ); // Can't use KontactApp since it's too late for adding cmdline options
  KTrader::OfferList offers = KTrader::self()->query(
    QString::fromLatin1( "Kontact/Plugin" ),
    QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  for ( KService::List::Iterator it = offers.begin(); it != offers.end(); ++it ) {
    KService::Ptr service = (*it);
    // skip summary only plugins
    QVariant var = service->property( "X-KDE-KontactPluginHasPart" );
    if ( var.isValid() && var.toBool() == false )
      continue;
    cout << service->library().remove( "libkontact_" ).latin1() << endl;
  }
}

static void listProfiles()
{
    KInstance *instance = new KInstance( "kontact" ); // Can't use KontactApp since it's too late for adding cmdline options
    QValueList<Kontact::Profile> profiles = Kontact::ProfileManager::self()->profiles();
    for( QValueListIterator<Kontact::Profile> it = profiles.begin() ; it != profiles.end(); ++it ) {
        cout << (*it).name().latin1() << endl;
    }
}

int KontactApp::newInstance()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString moduleName;
  if ( Kontact::Prefs::self()->forceStartupPlugin() ) {
    moduleName = Kontact::Prefs::self()->forcedStartupPlugin();
  }
  if ( args->isSet( "module" ) ) {
    moduleName = QString::fromLocal8Bit( args->getOption( "module" ) );
  }

  if ( !mSessionRestored ) {
    if ( !mMainWindow ) {
      mMainWindow = new Kontact::MainWindow();
      if ( !moduleName.isEmpty() )
        mMainWindow->setActivePluginModule( moduleName );
      mMainWindow->show();
      setMainWidget( mMainWindow );
      // --iconify is needed in kontact, although kstart can do that too,
      // because kstart returns immediately so it's too early to talk DCOP to the app.
      if ( args->isSet( "iconify" ) )
        KWin::iconifyWindow( mMainWindow->winId(), false /*no animation*/ );
    } else {
      if ( !moduleName.isEmpty() )
        mMainWindow->setActivePluginModule( moduleName );
    }
  }

    if ( args->isSet( "profile" ) ) {
        QValueList<Kontact::Profile>  profiles = Kontact::ProfileManager::self()->profiles();
        for( QValueListIterator<Kontact::Profile> it = profiles.begin(); it != profiles.end(); ++it ){
           if( args->getOption("profile") == (*it).name().latin1() ) {
             Kontact::ProfileManager::self()->loadProfile( (*it).id() );
             break;
           }
        }
    }

  AlarmClient alarmclient;
  alarmclient.startDaemon();

  // Handle startup notification and window activation
  // (The first time it will do nothing except note that it was called)
  return KUniqueApplication::newInstance();
}

int main( int argc, char **argv )
{
  KAboutData about( "kontact", I18N_NOOP( "Kontact" ), version, description,
                    KAboutData::License_GPL, I18N_NOOP("(C) 2001-2008 The Kontact developers"), 0, "http://kontact.org" );
  about.addAuthor( "Daniel Molkentin", 0, "molkentin@kde.org" );
  about.addAuthor( "Don Sanders", 0, "sanders@kde.org" );
  about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  about.addAuthor( "Tobias K\303\266nig", 0, "tokoe@kde.org" );
  about.addAuthor( "David Faure", 0, "faure@kde.org" );
  about.addAuthor( "Ingo Kl\303\266cker", 0, "kloecker@kde.org" );
  about.addAuthor( "Sven L\303\274ppken", 0, "sven@kde.org" );
  about.addAuthor( "Zack Rusin", 0, "zack@kde.org" );
  about.addAuthor( "Matthias Hoelzer-Kluepfel", I18N_NOOP("Original Author"), "mhk@kde.org" );

  KCmdLineArgs::init( argc, argv, &about );
  Kontact::UniqueAppHandler::loadKontactCommandLineOptions();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->isSet( "list" ) ) {
    listPlugins();
    return 0;
  }

  if ( args->isSet( "listprofiles" ) ) {
    listProfiles();
    return 0;
  }
 
  if ( !KontactApp::start() ) {
    // Already running, brought to the foreground.
    return 0;
  }

  KontactApp app;
  if ( app.restoringSession() ) {
     // There can only be one main window
    if ( KMainWindow::canBeRestored( 1 ) ) {
      Kontact::MainWindow *mainWindow = new Kontact::MainWindow();
      app.setMainWindow( mainWindow );
      app.setSessionRestored( true );
      mainWindow->show();
      mainWindow->restore( 1 );
    }
  }

  bool ret = app.exec();
  while ( KMainWindow::memberList->first() )
    delete KMainWindow::memberList->first();

  return ret;
}
