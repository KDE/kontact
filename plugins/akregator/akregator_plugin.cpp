/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QWidget>

#include <kaboutdata.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <core.h>
#include <plugin.h>
#include <kicon.h>
#include <akregator_options.h>
#include <akregator_part.h>
#include "akregator_plugin.h"
#include "partinterface.h"

namespace Akregator {

typedef KGenericFactory<Akregator::Plugin, Kontact::Core > PluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_akregator,
                            PluginFactory( "kontact_akregator" ) )

Plugin::Plugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "akregator" ), m_interface(0)
{

    setComponentData( PluginFactory::componentData() );

    KAction *action  = new KAction(KIcon("bookmark_add"), i18n("New Feed..."), this);
    actionCollection()->addAction("feed_new", action );
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F));
    connect(action, SIGNAL(triggered(bool)),SLOT( addFeed() ));
    insertNewAction(action);

    m_uniqueAppWatcher = new Kontact::UniqueAppWatcher(
	new Kontact::UniqueAppHandlerFactory<Akregator::UniqueAppHandler>(), this );
}

Plugin::~Plugin()
{
}

bool Plugin::isRunningStandalone()
{
    return m_uniqueAppWatcher->isRunningStandalone();
}

QStringList Plugin::invisibleToolbarActions() const
{
    return QStringList( "file_new_contact" );
}


OrgKdeAkregatorPartInterface *Plugin::interface()
{
  if(!m_interface)
    part();
  Q_ASSERT( m_interface );
  return m_interface;

}

QString Plugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "akregator/tips");
  QString file;
  return file;
}


MyBasePart* Plugin::createPart()
{
    MyBasePart* p = loadPart();

    connect(p, SIGNAL(showPart()), this, SLOT(showPart()));
    m_interface = new OrgKdeAkregatorPartInterface( "org.kde.akregator", "/Akregator", QDBusConnection::sessionBus() );

    m_interface->openStandardFeedList();
    return p;
}


void Plugin::showPart()
{
    core()->selectPlugin(this);
}

void Plugin::addFeed()
{
      m_interface->addFeed();
}

QStringList Plugin::configModules() const
{
    QStringList modules;
    modules << "PIM/akregator.desktop";
    return modules;
}

void Plugin::readProperties( KConfig *config )
{
    if ( part() ) {
        Akregator::Part *myPart = static_cast<Akregator::Part*>( part() );
        myPart->readProperties( config );
    }
}

void Plugin::saveProperties( KConfig *config )
{
    if ( part() ) {
        Akregator::Part *myPart = static_cast<Akregator::Part*>( part() );
        myPart->saveProperties( config );
    }
}

void UniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( akregator_options );
}

int UniqueAppHandler::newInstance()
{
    kDebug(5602) << k_funcinfo << endl;
    // Ensure part is loaded
    (void)plugin()->part();
#ifdef __GNUC__
#warning Port me to DBus!
#endif
//    DCOPRef akr( "akregator", "AkregatorIface" );
//    DCOPReply reply = kAB.call( "handleCommandLine" );
  //  if ( reply.isValid() ) {
    //    bool handled = reply;
     //   kDebug(5602) << k_funcinfo << "handled=" << handled << endl;
     //   if ( !handled ) // no args -> simply bring kaddressbook plugin to front
            return Kontact::UniqueAppHandler::newInstance();
   // }
   // return 0;
}

} // namespace Akregator
#include "akregator_plugin.moc"
