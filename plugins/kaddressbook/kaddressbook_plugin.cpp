#include <qwidget.h>


#include <kapplication.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include "kpcore.h"

#include "kaddressbook_plugin.h"
#include <kdebug.h>
#include "kaddressbook_plugin.moc"

typedef KGenericFactory< KAddressbookPlugin, Kaplan::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkaddressbookplugin,
                            KAddressbookPluginFactory( "kpaddressbookplugin" ) );

KAddressbookPlugin::KAddressbookPlugin(Kaplan::Core *_core, const char *name, const QStringList & /*args*/ )
  : Kaplan::Plugin(_core, _core, "kaddressbook"), m_part(0)
{
  m_stub = 0L;
  setInstance(KAddressbookPluginFactory::instance());

  setXMLFile("kpkaddressbookplugin.rc");


  core()->addMainEntry(i18n("Contacts"), "kaddressbook", this, SLOT(slotShowPlugin()));
  core()->insertNewAction( new KAction( i18n( "New Contact" ), BarIcon( "contact" ),
			  0, this, SLOT( slotNewContact() ), actionCollection(), "new_contact" ));
}


KAddressbookPlugin::~KAddressbookPlugin()
{
}

void KAddressbookPlugin::loadPart()
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
    // 1) Register with dcop as "kaddressbook"  [maybe the part should do this]
    // 2) Create the stub that allows us to talk to the part
    m_stub = new KAddressBookIface_stub(dcopClient(), "kaddressbook", "KAddressBookIface");
  }
}

void KAddressbookPlugin::slotShowPlugin()
{
  loadPart();
  if (m_part->widget())
    core()->showView(m_part->widget());
}

void KAddressbookPlugin::slotNewContact()
{
  loadPart();
  Q_ASSERT( m_stub );
  if ( m_stub )
      m_stub->newContact();
}

bool KAddressbookPlugin::createDCOPInterface( const QString& serviceType )
{
    if ( serviceType == "DCOP/AddressBook" )
    {
        loadPart();
        Q_ASSERT( m_stub );
        return true;
    }
    return false;
}
