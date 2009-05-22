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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "prefs.h"
#include "reminderclient.h"
#include "mainwindow.h"
#include "plugin.h"
#include "uniqueapphandler.h"

#include <pimapplication.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstartupinfo.h>
#include <kuniqueapplication.h>
#include <kwindowsystem.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kservicetypetrader.h>

#include <QLabel>

#include <iostream>
#include "profilemanager.h"

using namespace std;

static const char description[] =
  I18N_NOOP( "KDE personal information manager" );

static const char version[] = "1.3 (enterprise4 0.20090522.971344)";

class KontactApp : public
#ifdef Q_WS_WIN
KPIM::PimApplication
#else
KUniqueApplication
#endif
{
  Q_OBJECT
  public:
    KontactApp() : mMainWindow( 0 ), mSessionRestored( false ) {
      KIconLoader::global()->addAppDir( "kdepim" );
    }
    ~KontactApp() {}

    /*reimp*/ int newInstance();

    void setMainWindow( Kontact::MainWindow *window ) {
        mMainWindow = window;
        Kontact::UniqueAppHandler::setMainWidget( window );
    }
    void setSessionRestored( bool restored ) {
        mSessionRestored = restored;
    }

  public Q_SLOTS:
    void loadCommandLineOptionsForNewInstance();

  private:
    Kontact::MainWindow *mMainWindow;
    bool mSessionRestored;
};

static void listPlugins()
{
  KComponentData instance( "kontact" ); //Can't use KontactApp -- too late for adding cmdline opts

  KService::List offers = KServiceTypeTrader::self()->query(
    QString::fromLatin1( "Kontact/Plugin" ),
    QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  for ( KService::List::Iterator it = offers.begin(); it != offers.end(); ++it ) {
    KService::Ptr service = (*it);
    // skip summary only plugins
    QVariant var = service->property( "X-KDE-KontactPluginHasPart" );
    if ( var.isValid() && var.toBool() == false ) {
      continue;
    }
    cout << service->library().remove( "kontact_" ).toLatin1().data() << endl;
  }
}

static void loadCommandLineOptions()
{
  KCmdLineOptions options;
  options.add( "module <module>", ki18n( "Start with a specific Kontact module" ) );
  options.add( "iconify", ki18n( "Start in iconified (minimized) mode" ) );
  options.add( "list", ki18n( "List all possible modules and exit" ) );
  options.add( "listprofiles", ki18n( "List all possible profiles and exit" ) );
  options.add( "profile <profile>", ki18n( "Start with a specific Kontact profile" ) );
  KCmdLineArgs::addCmdLineOptions( options );
}

// Called by KUniqueApplication
void KontactApp::loadCommandLineOptionsForNewInstance()
{
  kDebug();
  KCmdLineArgs::reset(); // forget options defined by other "applications"
  loadCommandLineOptions(); // re-add the kontact options
}

static void listProfiles()
{
    KComponentData instance( "kontact" ); // Can't use KontactApp since it's too late for adding cmdline options
    QList<Kontact::Profile> profiles = Kontact::ProfileManager::self()->profiles();
    for( QList<Kontact::Profile>::iterator it = profiles.begin() ; it != profiles.end(); ++it ) {
        cout << (*it).name().toLatin1().data() << endl;
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
    moduleName = args->getOption( "module" );
  }
  if ( !mSessionRestored ) {
    if ( !mMainWindow ) {
      mMainWindow = new Kontact::MainWindow();
      if ( !moduleName.isEmpty() ) {
        mMainWindow->setActivePluginModule( moduleName );
      }
      mMainWindow->show();
      Kontact::UniqueAppHandler::setMainWidget( mMainWindow );
      // --iconify is needed in kontact, although kstart can do that too,
      // because kstart returns immediately so it's too early to talk D-Bus to the app.
      if ( args->isSet( "iconify" ) ) {
        KWindowSystem::minimizeWindow( mMainWindow->winId(), false /*no animation*/);
      }
    } else {
      if ( !moduleName.isEmpty() ) {
        mMainWindow->setActivePluginModule( moduleName );
      }
    }
  }

  if ( args->isSet( "profile" ) ) {
    QList<Kontact::Profile>  profiles = Kontact::ProfileManager::self()->profiles();
    for( QList<Kontact::Profile>::iterator it = profiles.begin(); it != profiles.end(); ++it ){
      if( args->getOption("profile") == (*it).name().toLatin1() ) {
        Kontact::ProfileManager::self()->loadProfile( (*it).id() );
        break;
      }
    }
  }

  KPIM::ReminderClient reminderclient;
  reminderclient.startDaemon();

  // Handle startup notification and window activation
  // (The first time it will do nothing except note that it was called)
  return KUniqueApplication::newInstance();
}

int main( int argc, char **argv )
{
  KAboutData about( "kontact", 0, ki18n( "Kontact" ), version, ki18n(description),
                    KAboutData::License_GPL,
                    ki18n( "(C) 2001-2008 The Kontact developers" ),
                    KLocalizedString(), "http://kontact.org" );

  about.addAuthor( ki18n( "Allen Winter" ), KLocalizedString(), "winter@kde.org" );
  about.addAuthor( ki18n( "Rafael Fernández López" ), KLocalizedString(), "ereslibre@kde.org" );
  about.addAuthor( ki18n( "Daniel Molkentin" ), KLocalizedString(), "molkentin@kde.org" );
  about.addAuthor( ki18n( "Don Sanders" ), KLocalizedString(), "sanders@kde.org" );
  about.addAuthor( ki18n( "Cornelius Schumacher" ), KLocalizedString(), "schumacher@kde.org" );
  about.addAuthor( ki18n( "Tobias K\303\266nig" ), KLocalizedString(), "tokoe@kde.org" );
  about.addAuthor( ki18n( "David Faure" ), KLocalizedString(), "faure@kde.org" );
  about.addAuthor( ki18n( "Ingo Kl\303\266cker" ), KLocalizedString(), "kloecker@kde.org" );
  about.addAuthor( ki18n( "Sven L\303\274ppken" ), KLocalizedString(), "sven@kde.org" );
  about.addAuthor( ki18n( "Zack Rusin" ), KLocalizedString(), "zack@kde.org" );
  about.addAuthor( ki18n( "Matthias Hoelzer-Kluepfel" ),
                   ki18n( "Original Author" ), "mhk@kde.org" );
  about.setOrganizationDomain( "kde.org" );

  KCmdLineArgs::init( argc, argv, &about );

  loadCommandLineOptions();
  KUniqueApplication::addCmdLineOptions();
  KCmdLineArgs::addStdCmdLineOptions();

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

  // Qt doesn't treat the system tray as a window, and therefore Qt would quit
  // the event loop when an error message is clicked away while Kontact is in the
  // tray.
  // Rely on KGlobal::ref() and KGlobal::deref() instead, like we did in KDE3.
  // See http://bugs.kde.org/show_bug.cgi?id=163479
  QApplication::setQuitOnLastWindowClosed( false );

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
  qDeleteAll( KMainWindow::memberList() );

  return ret;
}

#include "main.moc"
