#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include "kpcore.h"


#include "korganizer_plugin.h"

typedef KGenericFactory< KOrganizerPlugin, Kaplan::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkorganizerplugin, 
                            KOrganizerPluginFactory( "kporganizerplugin" ) );

KOrganizerPlugin::KOrganizerPlugin(Kaplan::Core *_core, const char *, const QStringList &)
  : Kaplan::Plugin(_core, _core, "korganizer"), m_part(0)
{
  setInstance(KOrganizerPluginFactory::instance());

  setXMLFile("kpkorganizerplugin.rc");

  core()->addMainEntry(i18n("Dates"), "korganizer", this, SLOT(slotShowPlugin()));
  core()->insertNewAction(  new KAction(  i18n(  "New Appointment" ), BarIcon(  "appointment" ),
			                0, this, SLOT(  slotNewAppointment() ), actionCollection(), "new_appointment" ) );
 
  m_iface = new KOrganizerIface_stub(dcopClient(), "korganizer", "KOrganizerIface"); 
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

void KOrganizerPlugin::slotNewAppointment()
{
//	m_iface->newAppointment(); ###Add Method to Iface
}

#include "korganizer_plugin.moc"
