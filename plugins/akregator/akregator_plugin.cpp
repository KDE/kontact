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

#include <kontact/core.h>
#include <kontact/plugin.h>

#include "akregator_options.h"
#include "akregator_plugin.h"

typedef KGenericFactory< aKregatorPlugin, Kontact::Core > aKregatorPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_akregator,
                            aKregatorPluginFactory( "kontact_akregator" ) )

aKregatorPlugin::aKregatorPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "akregator" ), m_stub(0)
{

    setInstance( aKregatorPluginFactory::instance() );
    
    m_uniqueAppWatcher = new Kontact::UniqueAppWatcher(
            new Kontact::UniqueAppHandlerFactory<AkregatorUniqueAppHandler>(), this );
}

aKregatorPlugin::~aKregatorPlugin()
{
}

bool aKregatorPlugin::isRunningStandalone()
{
    return m_uniqueAppWatcher->isRunningStandalone();
}

QStringList aKregatorPlugin::invisibleToolbarActions() const
{
    return QStringList( "file_new_contact" );
}


Akregator::aKregatorPartIface_stub *aKregatorPlugin::interface()
{
    if ( !m_stub ) {
        part();
    }

    Q_ASSERT( m_stub );
    return m_stub;
}


MyBasePart* aKregatorPlugin::createPart()
{
    MyBasePart* p = loadPart();

    connect(p, SIGNAL(showPart()), this, SLOT(showPart()));
    m_stub = new Akregator::aKregatorPartIface_stub( dcopClient(), "akregator",
                                      "aKregatorIface" );
    m_stub->openStandardFeedList();
    return p;
}


void aKregatorPlugin::showPart()
{
    core()->selectPlugin(this);
}


QStringList aKregatorPlugin::configModules() const
{
    QStringList modules;
    modules << "PIM/akregator.desktop";
    return modules;
}

void AkregatorUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( akregator_options );
}

int AkregatorUniqueAppHandler::newInstance()
{
    kdDebug(5602) << k_funcinfo << endl;
    // Ensure part is loaded
    (void)plugin()->part();
    DCOPRef akr( "akregator", "aKregatorIface" );
//    DCOPReply reply = kAB.call( "handleCommandLine" );
  //  if ( reply.isValid() ) {
    //    bool handled = reply;
     //   kdDebug(5602) << k_funcinfo << "handled=" << handled << endl;
     //   if ( !handled ) // no args -> simply bring kaddressbook plugin to front
            return Kontact::UniqueAppHandler::newInstance();
   // }
   // return 0;
}


#include "akregator_plugin.moc"
