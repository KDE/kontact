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

// Use the kdepim version
#include "kdepim-version.h"

#include "mainwindow.h"
#include "prefs.h"
using namespace Kontact;

#include <libkdepimdbusinterfaces/reminderclient.h>

#include <KontactInterface/Plugin>
#include <KontactInterface/UniqueAppHandler>
#ifdef Q_WS_WIN
#include <KontactInterface/PimUniqueApplication>
#endif

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KService>
#include <KServiceTypeTrader>
#include <KUniqueApplication>
#include <KWindowSystem>

#include <iostream>
using namespace std;

static const char description[] = I18N_NOOP( "KDE personal information manager" );

static const char version[] = KDEPIM_VERSION;

class KontactApp : public
#ifdef Q_WS_WIN
KontactInterface::PimUniqueApplication
#else
KUniqueApplication
#endif
{
  Q_OBJECT
  public:
    KontactApp() : mMainWindow( 0 ), mSessionRestored( false )
    {
      KIconLoader::global()->addAppDir( QLatin1String("kdepim") );
    }
    ~KontactApp() {}

    /*reimp*/
    int newInstance();

    void setMainWindow( MainWindow *window )
    {
      mMainWindow = window;
      KontactInterface::UniqueAppHandler::setMainWidget( window );
    }
    void setSessionRestored( bool restored )
    {
      mSessionRestored = restored;
    }

  public Q_SLOTS:
    void loadCommandLineOptionsForNewInstance();

  private:
    MainWindow *mMainWindow;
    bool mSessionRestored;
};

static void listPlugins()
{
  KComponentData instance( "kontact" ); //Can't use KontactApp -- too late for adding cmdline opts

  const KService::List offers = KServiceTypeTrader::self()->query(
    QString::fromLatin1( "Kontact/Plugin" ),
    QString::fromLatin1( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  KService::List::ConstIterator end( offers.end() );
  for ( KService::List::ConstIterator it = offers.begin(); it != end; ++it ) {
    KService::Ptr service = (*it);
    // skip summary only plugins
    QVariant var = service->property( QLatin1String("X-KDE-KontactPluginHasPart") );
    if ( var.isValid() && var.toBool() == false ) {
      continue;
    }
    cout << service->library().remove( QLatin1String("kontact_") ).toLatin1().data() << endl;
  }
}

static void loadCommandLineOptions()
{
  KCmdLineOptions options;
  options.add( "module <module>", ki18n( "Start with a specific Kontact module" ) );
  options.add( "iconify", ki18n( "Start in iconified (minimized) mode" ) );
  options.add( "list", ki18n( "List all possible modules and exit" ) );
  KCmdLineArgs::addCmdLineOptions( options );
}

// Called by KUniqueApplication
void KontactApp::loadCommandLineOptionsForNewInstance()
{
  KCmdLineArgs::reset(); // forget options defined by other "applications"
  loadCommandLineOptions(); // re-add the kontact options
}

int KontactApp::newInstance()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString moduleName;
  if ( Prefs::self()->forceStartupPlugin() ) {
    moduleName = Prefs::self()->forcedStartupPlugin();
  }
  if ( args->isSet( "module" ) ) {
    moduleName = args->getOption( "module" );
  }
  if ( !mSessionRestored ) {
    if ( !mMainWindow ) {
      mMainWindow = new MainWindow();
      if ( !moduleName.isEmpty() ) {
        mMainWindow->setInitialActivePluginModule( moduleName );
      }
      mMainWindow->show();
      KontactInterface::UniqueAppHandler::setMainWidget( mMainWindow );
      // --iconify is needed in kontact, although kstart can do that too,
      // because kstart returns immediately so it's too early to talk D-Bus to the app.
      if ( args->isSet( "iconify" ) ) {
        KWindowSystem::minimizeWindow( mMainWindow->winId(), false /*no animation*/);
      }
    } else {
      if ( !moduleName.isEmpty() ) {
        mMainWindow->setInitialActivePluginModule( moduleName );
      }
    }
  }

  KPIM::ReminderClient::startDaemon();

  // Handle startup notification and window activation
  // (The first time it will do nothing except note that it was called)
  return KUniqueApplication::newInstance();
}

int main( int argc, char **argv )
{
  KAboutData about( "kontact", 0, ki18n( "Kontact" ), version, ki18n(description),
                    KAboutData::License_GPL,
                    ki18n( "Copyright © 2001–2013 Kontact authors" ),
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
  about.addCredit( ki18n( "Torgny Nyblom" ), ki18n("Git Migration"), "nyblom@kde.org" );
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
      MainWindow *mainWindow = new MainWindow();
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
