#include <qwidget.h>


#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include "kpcore.h"

#include "kmail_plugin.h"
#include "kmail_plugin.moc"

typedef KGenericFactory< KMailPlugin, Kontact::Core > KMailPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkmailplugin,
                            KMailPluginFactory( "kpmailplugin" ) );

KMailPlugin::KMailPlugin(Kontact::Core *_core, const char * /*name*/, const QStringList & /*args*/ )
  : Kontact::Plugin(i18n("Mail"), "kmail", _core, _core,  "kmail"), m_part(0)
{

  setInstance(KMailPluginFactory::instance());

  setXMLFile("kpkmailplugin.rc");

}


KMailPlugin::~KMailPlugin()
{
}

bool KMailPlugin::createDCOPInterface( const QString& serviceType )
{
  if ( serviceType == "DCOP/ResourceBackend/IMAP" )
  {
    if ( part() )
      return true;
  }

  return false;
}

KParts::Part* KMailPlugin::part()
{
  m_stub = new KMailPartIface_stub(dcopClient(), "kmail", "*");
  if (!m_part)
  {
    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkmailpart",
                                                             core(), 0, // parentwidget,name
                                                             this, 0 ); // parent,name
    if (!m_part)
      return 0;

    return m_part;
  }
  else
	return m_part;
}

