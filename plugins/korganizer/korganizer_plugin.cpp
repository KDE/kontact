#include <qwidget.h>


#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include "kpcore.h"

#include "korganizer_plugin.h"
#include <kdebug.h>

typedef KGenericFactory< KOrganizerPlugin, Kontact::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkorganizerplugin,
                            KOrganizerPluginFactory( "kporganizerplugin" ) );

KOrganizerPlugin::KOrganizerPlugin(Kontact::Core *_core, const char *, const QStringList &)
  : Kontact::Plugin(i18n("Calendar"), "korganizer", _core, _core, "korganizer"), m_part(0)
{
  setInstance(KOrganizerPluginFactory::instance());

  setXMLFile("kpkorganizerplugin.rc");

  insertNewAction(  new KAction(  i18n(  "New Appointment" ), BarIcon(  "appointment" ),
			                0, this, SLOT(  slotNewAppointment() ), actionCollection(), "new_appointment" ) );

  m_iface = 0L;
}


KOrganizerPlugin::~KOrganizerPlugin()
{
}

KParts::ReadOnlyPart* KOrganizerPlugin::part()
{
    (void) dcopClient(); // ensure that we register to DCOP as "korganizer"
    m_iface = new KOrganizerIface_stub(dcopClient(), "korganizer", "KOrganizerIface");

    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkorganizer",
                                                               0, 0, // parentwidget,name
                                                               this, 0 ); // parent,name
    return m_part;

}

void KOrganizerPlugin::slotNewAppointment()
{
  if ( !m_iface)
    return;
//  m_iface->newAppointment(); //### Add Method to Iface
}

bool KOrganizerPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" )
  {
    if ( part() )
      return true;
  }

  return false;
}

#include "korganizer_plugin.moc"
