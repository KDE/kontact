#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>


#include "kpcore.h"
#include "kpfactory.h"


#include "kaddressbook_plugin.h"
#include "kaddressbook_plugin.moc"


KAddressbookPlugin::KAddressbookPlugin(Kaplan::Core *_core, QObject *parent, const char *name)
  : Kaplan::Plugin(_core, parent, name), m_part(0)
{
  setInstance(Kaplan::FactoryImpl<KAddressbookPlugin>::instance("kpkaddressbookplugin"));

  setXMLFile("kpkaddressbookplugin.rc");

  core()->addMainEntry("Contacts", "kaddressbook", this, SLOT(slotShowPlugin()));
}


KAddressbookPlugin::~KAddressbookPlugin()
{
}


void KAddressbookPlugin::slotShowPlugin()
{
  if (!m_part)
  {
    KLibFactory *factory = KLibLoader::self()->factory("libkaddressbook");
    if (!factory)
      return;

    QObject *obj = factory->create();
    if (!obj || !obj->inherits("KParts::ReadOnlyPart")) 
      return;
    
    m_part = static_cast<KParts::ReadOnlyPart*>(obj);
    core()->addPart(m_part);
  }

  if (m_part->widget()) 
    core()->showView(m_part->widget()); 
}


extern "C"
{

  void *init_libkpkaddressbookplugin()
  {
    return new Kaplan::FactoryImpl<KAddressbookPlugin>;
  };
  
}
