#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>


#include "kpcore.h"


#include "kaddressbook_plugin.h"
#include "kaddressbook_plugin.moc"

typedef KGenericFactory< KAddressbookPlugin, Kaplan::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkaddressbookplugin, 
                            KAddressbookPluginFactory( "kpaddressbookplugin" ) );

KAddressbookPlugin::KAddressbookPlugin(Kaplan::Core *_core, const char *name, const QStringList & /*args*/ )
  : Kaplan::Plugin(_core, _core, name), m_part(0)
{
  setInstance(KAddressbookPluginFactory::instance());

  setXMLFile("kpkaddressbookplugin.rc");

  core()->addMainEntry(i18n("Contacts"), "kaddressbook", this, SLOT(slotShowPlugin()));
}


KAddressbookPlugin::~KAddressbookPlugin()
{
}


void KAddressbookPlugin::slotShowPlugin()
{
  if (!m_part)
  {
    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkaddressbookpart",
                                                             0, 0, // parentwidget,name
                                                             this, 0 ); // parent,name
    if (!m_part)
      return;

    core()->addPart(m_part);
  }

  if (m_part->widget()) 
    core()->showView(m_part->widget()); 
}

