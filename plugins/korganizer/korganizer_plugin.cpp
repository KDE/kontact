#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>

#include "kpcore.h"


#include "korganizer_plugin.h"

typedef KGenericFactory< KOrganizerPlugin, Kaplan::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkorganizerplugin, 
                            KOrganizerPluginFactory( "kporganizerplugin" ) );

KOrganizerPlugin::KOrganizerPlugin(Kaplan::Core *_core, const char *name, const QStringList &)
  : Kaplan::Plugin(_core, _core, name), m_part(0)
{
  setInstance(KOrganizerPluginFactory::instance());

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
    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkorganizer",
                                                               0, 0, // parentwidget,name
                                                               this, 0 ); // parent,name
    core()->addPart(m_part);
  }

  if (m_part->widget()) 
    core()->showView(m_part->widget()); 
}


#include "korganizer_plugin.moc"
