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
#include <ktrader.h>
#include "plugin.h"

#include <qlabel.h>
#include "splash.h"

#include "mainwindow.h"

using namespace std;

static const char description[] =
    I18N_NOOP( "KDE personal information manager" );

static const char version[] = "1.0";

class KontactApp : public KUniqueApplication {
  public:
    KontactApp() : mMainWindow( 0 ) {}
    ~KontactApp() {}

    int newInstance();

  private:
    Kontact::MainWindow *mMainWindow;
};

static void listPlugins()
{
  KInstance instance( "kontact" ); // Can't use KontactApp since it's too late for adding cmdline options
  KTrader::OfferList offers = KTrader::self()->query(
    QString::fromLatin1( "Kontact/Plugin" ),
    QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );
  for(KService::List::Iterator it = offers.begin(); it != offers.end(); ++it)
  {
    KService::Ptr service = (*it);
    cout << service->library().remove( "libkontact_" ).latin1() << endl;
  }
}

static KCmdLineOptions options[] =
{
    { "module <module>",   I18N_NOOP("Start with a specific Kontact module"), 0 },
    { "nosplash",   I18N_NOOP("Disable the splash screen"), 0 },
    { "list", I18N_NOOP("List all possible modules and exit"), 0 },
    KCmdLineLastOption
};


int KontactApp::newInstance()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString moduleName;
  if ( args->isSet("module") )
  {
    moduleName = QString::fromLocal8Bit(args->getOption("module"));
  }
  Kontact::Splash* splash = new Kontact::Splash( 0, "splash" );
  if ( !mMainWindow && args->isSet("splash") ) // only the first time
    splash->show();

  if ( isRestored() ) {
    // There can only be one main window
    if ( KMainWindow::canBeRestored( 1 ) ) {
      mMainWindow = new Kontact::MainWindow(splash);
      setMainWidget( mMainWindow );
      mMainWindow->show();
      mMainWindow->restore( 1 );
    }
  } else {
    if ( !mMainWindow ) {
      mMainWindow = new Kontact::MainWindow(splash);
      if ( !moduleName.isEmpty() )
        mMainWindow->activePluginModule( moduleName );
      mMainWindow->show();
      setMainWidget( mMainWindow );
    }
    else
    {
      if ( !moduleName.isEmpty() )
        mMainWindow->activePluginModule( moduleName );
    }
  }

  // Handle startup notification and window activation
  // (The first time it will do nothing except note that it was called)
  return KUniqueApplication::newInstance();
}

int main(int argc, char **argv)
{
  KAboutData about( "kontact", I18N_NOOP( "Kontact" ), version, description,
                    KAboutData::License_GPL, I18N_NOOP("(C) 2001-2004 The Kontact developers"), 0, "http://kontact.org" );
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
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();
  KApplication::addCmdLineOptions();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->isSet( "list" ) )
  {
    listPlugins();
    return 0;
  }

  if ( !KontactApp::start() ) {
    // Already running, brought to the foreground.
    return 0;
  }

  KontactApp app;
  bool ret = app.exec();
  while ( KMainWindow::memberList->first() )
    delete KMainWindow::memberList->first();

  return ret;
}
