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

typedef KGenericFactory< KMailPlugin, Kaplan::Core > KMailPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkmailplugin,
                            KMailPluginFactory( "kpmailplugin" ) );

KMailPlugin::KMailPlugin(Kaplan::Core *_core, const char *name, const QStringList & /*args*/ )
  : Kaplan::Plugin(_core, _core, "kmail"), m_part(0)
{
//  m_stub = new KMailPartIface_stub(dcopClient(), "kmail", "*"); 	
  setInstance(KMailPluginFactory::instance());

  setXMLFile("kpkmailplugin.rc");

  core()->addMainEntry(i18n("Mail"), "kmail", this, SLOT(slotShowPlugin()));
}


KMailPlugin::~KMailPlugin()
{
}

void KMailPlugin::loadPart()
{
  if (!m_part)
  {
    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkmailpart",
                                                             0, 0, // parentwidget,name
                                                             this, 0 ); // parent,name
    if (!m_part)
      return;

    core()->addPart(m_part);
  }
}

void KMailPlugin::slotShowPlugin()
{
  loadPart();
  if (m_part->widget())
    core()->showView(m_part->widget());
}


