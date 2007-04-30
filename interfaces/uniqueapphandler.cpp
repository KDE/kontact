/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "uniqueapphandler.h"
#include <kstartupinfo.h>
#include <kcmdlineargs.h>
#include "core.h"
#include <kwindowsystem.h>
#include <kdebug.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

/*
 Test plan for the various cases of interaction between standalone apps and kontact:

 1) start kontact, select "Mail".
 1a) type "korganizer" -> it switches to korganizer
 1b) type "kmail" -> it switches to kmail
 1c) type "kaddressbook" -> it switches to kaddressbook
 1d) type "kmail foo@kde.org" -> it opens a kmail composer, without switching
 1e) type "knode" -> it switches to knode
 1f) type "kaddressbook --new-contact" -> it opens a kaddressbook contact window
 1g) type "knode news://foobar/group" -> it pops up "can't resolve hostname"

 2) close kontact. Launch kmail. Launch kontact again.
 2a) click "Mail" icon -> kontact doesn't load a part, but activates the kmail window
 2b) type "kmail foo@kde.org" -> standalone kmail opens composer.
 2c) close kmail, click "Mail" icon -> kontact loads the kmail part.
 2d) type "kmail" -> kontact is brought to front

 3) close kontact. Launch korganizer, then kontact.
 3a) both Todo and Calendar activate the running korganizer.
 3b) type "korganizer" -> standalone korganizer is brought to front
 3c) close korganizer, click Calendar or Todo -> kontact loads part.
 3d) type "korganizer" -> kontact is brought to front

 4) close kontact. Launch kaddressbook, then kontact.
 4a) "Contacts" icon activate the running kaddressbook.
 4b) type "kaddressbook" -> standalone kaddressbook is brought to front
 4c) close kaddressbook, type "kaddressbook -a foo@kde.org" -> kontact loads part and opens editor
 4d) type "kaddressbook" -> kontact is brought to front

 5) close kontact. Launch knode, then kontact.
 5a) "News" icon activate the running knode.
 5b) type "knode" -> standalone knode is brought to front
 5c) close knode, type "knode news://foobar/group" -> kontact loads knode and pops up msgbox
 5d) type "knode" -> kontact is brought to front

 6) start "kontact --module summaryplugin"
 6a) type "dcop kmail kmail newInstance" -> kontact switches to kmail (#103775)
 6b) type "kmail" -> kontact is brought to front
 6c) type "kontact" -> kontact is brought to front
 6d) type "kontact --module summaryplugin" -> kontact switches to summary

*/

using namespace Kontact;

UniqueAppHandler::UniqueAppHandler( Plugin* plugin ) 
 : mPlugin( plugin ) 
{
  kDebug()<< k_funcinfo <<" plugin->objectName().toLatin1() :"<<plugin->objectName().toLatin1()<<endl; 
  QDBusConnection::sessionBus().registerService( "org.kde." + plugin->objectName().toLatin1() );
}

UniqueAppHandler::~UniqueAppHandler()
{
}

int UniqueAppHandler::newInstance()
{
  // This bit is duplicated from KUniqueApplication::newInstance()
  if ( kapp->mainWidget() ) {
    kapp->mainWidget()->show();
#ifdef Q_OS_UNIX    
    KWindowSystem::forceActiveWindow( kapp->mainWidget()->winId() );
#endif    
    KStartupInfo::appStarted();
  }

  // Then ensure the part appears in kontact
  mPlugin->core()->selectPlugin( mPlugin );
  return 0;
}

#ifdef __GNUC__
#warning Port to DBus!
#endif
/*bool UniqueAppHandler::process( const DCOPCString &fun, const QByteArray &data,
                                DCOPCString& replyType, QByteArray &replyData )
{
  if ( fun == "newInstance()" ) {
    replyType = "int";

    KCmdLineArgs::reset(); // forget options defined by other "applications"
    loadCommandLineOptions(); // implemented by plugin

    // This bit is duplicated from KUniqueApplication::processDelayed()
    QByteArray tmp(data);
	QDataStream ds( &tmp,QIODevice::ReadOnly );
    ds.setVersion(QDataStream::Qt_3_1);
    KCmdLineArgs::loadAppArgs( ds );
    if ( !ds.atEnd() ) { // backwards compatibility
      QByteArray asn_id;
      ds >> asn_id;
      kapp->setStartupId( asn_id );
    }

    QDataStream _replyStream( &replyData,QIODevice::WriteOnly );
    _replyStream.setVersion(QDataStream::Qt_3_1);
    _replyStream << newInstance( );
  } else if ( fun == "load()" ) {
    replyType = "bool";
    (void)mPlugin->part(); // load the part without bringing it to front

    QDataStream _replyStream( &replyData,QIODevice::WriteOnly );
    _replyStream.setVersion(QDataStream::Qt_3_1);
    _replyStream << true;
  } else {
    return DCOPObject::process( fun, data, replyType, replyData );
  }
  return true;
}

DCOPCStringList UniqueAppHandler::interfaces()
{
  DCOPCStringList ifaces = DCOPObject::interfaces();
  ifaces += "Kontact::UniqueAppHandler";
  return ifaces;
}

DCOPCStringList UniqueAppHandler::functions()
{
  DCOPCStringList funcs = DCOPObject::functions();
  funcs << "int newInstance()";
  funcs << "bool load()";
  return funcs;
}*/

UniqueAppWatcher::UniqueAppWatcher( UniqueAppHandlerFactoryBase* factory, Plugin* plugin )
    : QObject( plugin ), mFactory( factory ), mPlugin( plugin )
{
  // The app is running standalone if 1) that name is known to D-Bus
    QString serviceName = "org.kde."+plugin->objectName().toLatin1();
    mRunningStandalone = QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName);
    kDebug()<<" plugin->objectName() :"<<plugin->objectName()<<" isServiceRegistered ? :"<<mRunningStandalone<<endl;
 
    QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(serviceName); 
    if( mRunningStandalone && (owner == QDBusConnection::sessionBus().baseService())) 
       mRunningStandalone = false;

    if(mRunningStandalone)
    {
      //TODO port it
      //kapp->dcopClient()->setNotifications( true );
      //connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QByteArray& ) ), this, SLOT( unregisteredFromDCOP( const QByteArray& ) ) );
    }
    else
	mFactory->createHandler( mPlugin ); 
}

UniqueAppWatcher::~UniqueAppWatcher()
{
#ifdef __GNUC__
#warning Port to DBus!
#endif
//  if ( mRunningStandalone )
//    kapp->dcopClient()->setNotifications( false );

  delete mFactory;
}

void UniqueAppWatcher::unregisteredFromDCOP( const QByteArray& appId )
{
  if ( appId == mPlugin->objectName() && mRunningStandalone ) {
#ifdef __GNUC__
#warning Port to DBus!
#endif
//    disconnect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QByteArray& ) ),
//                this, SLOT( unregisteredFromDCOP( const QByteArray& ) ) );
    kDebug(5601) << k_funcinfo << appId << endl;
    mFactory->createHandler( mPlugin );
//    kapp->dcopClient()->setNotifications( false );
    mRunningStandalone = false;
  }
}

static KCmdLineOptions options[] =
{
    { "module <module>",   I18N_NOOP( "Start with a specific Kontact module" ), 0 },
    { "iconify",   I18N_NOOP( "Start in iconified (minimized) mode" ), 0 },
    { "list", I18N_NOOP( "List all possible modules and exit" ), 0 },
    KCmdLineLastOption
};

void Kontact::UniqueAppHandler::loadKontactCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();
  KCmdLineArgs::addStdCmdLineOptions();
}

#include "uniqueapphandler.moc"
