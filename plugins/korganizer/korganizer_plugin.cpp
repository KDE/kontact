#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>


#include "kpcore.h"
#include "kpfactory.h"


#include "korganizer_plugin.h"


KOrganizerPlugin::KOrganizerPlugin(Kaplan::Core *_core, QObject *parent, const char *name)
  : Kaplan::Plugin(_core, parent, name), m_part(0)
{
  setInstance(Kaplan::FactoryImpl<KOrganizerPlugin>::instance("kpkorganizerplugin"));

  setXMLFile("kpkorganizerplugin.rc");

  core()->addMainEntry("Dates", "korganizer", this, SLOT(slotShowPlugin()));
}


KOrganizerPlugin::~KOrganizerPlugin()
{
}


void KOrganizerPlugin::slotShowPlugin()
{
  if (!m_part)
  {
    KLibFactory *factory = KLibLoader::self()->factory("libkorganizer");
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

  void *init_libkpkorganizerplugin()
  {
    return new Kaplan::FactoryImpl<KOrganizerPlugin>;
  };
  
}

#include "korganizer_plugin.moc"
