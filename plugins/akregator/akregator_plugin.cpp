/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#include <qwidget.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include <kontact/core.h>
#include <kontact/plugin.h>

#include "akregator_plugin.h"

typedef KGenericFactory< aKregatorPlugin, Kontact::Core > aKregatorPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_akregator,
                            aKregatorPluginFactory( "kontact_akregator" ) )

aKregatorPlugin::aKregatorPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "akregator" ), m_stub(0)
{
     setInstance( aKregatorPluginFactory::instance() );
}

aKregatorPlugin::~aKregatorPlugin()
{
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


KPIM::Part* aKregatorPlugin::createPart()
{
    KPIM::Part* p = loadPart();

    m_stub = new Akregator::aKregatorPartIface_stub( dcopClient(), "akregator",
                                      "aKregatorIface" );
    m_stub->openStandardFeedList();
    return p;
}

QStringList aKregatorPlugin::configModules() const
{
    QStringList modules;
    modules << "PIM/akregator.desktop";
    return modules;
}

#include "akregator_plugin.moc"
