/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#include <qwidget.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <core.h>
#include <plugin.h>

#include <akregator_options.h>
#include "akregator_plugin.h"
namespace Akregator {

typedef KGenericFactory<Akregator::Plugin, Kontact::Core > PluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_akregator,
                            PluginFactory( "kontact_akregator" ) )

Plugin::Plugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "akregator" ), m_stub(0)
{

    setInstance( PluginFactory::instance() );
    
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


Akregator::AkregatorPartIface_stub *Plugin::interface()
{
    if ( !m_stub ) {
        part();
    }

    Q_ASSERT( m_stub );
    return m_stub;
}


MyBasePart* Plugin::createPart()
{
    MyBasePart* p = loadPart();

    connect(p, SIGNAL(showPart()), this, SLOT(showPart()));
    m_stub = new Akregator::AkregatorPartIface_stub( dcopClient(), "akregator",
                                      "AkregatorIface" );
    m_stub->openStandardFeedList();
    return p;
}


void Plugin::showPart()
{
    core()->selectPlugin(this);
}


QStringList Plugin::configModules() const
{
    QStringList modules;
    modules << "PIM/akregator.desktop";
    return modules;
}

void UniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( akregator_options );
}

int UniqueAppHandler::newInstance()
{
    kdDebug(5602) << k_funcinfo << endl;
    // Ensure part is loaded
    (void)plugin()->part();
    DCOPRef akr( "akregator", "AkregatorIface" );
//    DCOPReply reply = kAB.call( "handleCommandLine" );
  //  if ( reply.isValid() ) {
    //    bool handled = reply;
     //   kdDebug(5602) << k_funcinfo << "handled=" << handled << endl;
     //   if ( !handled ) // no args -> simply bring kaddressbook plugin to front
            return Kontact::UniqueAppHandler::newInstance();
   // }
   // return 0;
}

} // namespace Akregator
#include "akregator_plugin.moc"
