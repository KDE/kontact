#include <qwidget.h>


#include <kapplication.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>

#include <kdebug.h>

#include "kpcore.h"
#include "kpplugin.h"

#include "kabsummarywidget.h"
#include "kaddressbook_plugin.h"
#include "kaddressbook_plugin.moc"

typedef KGenericFactory< KAddressbookPlugin, Kontact::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkpkaddressbookplugin,
                            KAddressbookPluginFactory( "kpaddressbookplugin" ) );

KAddressbookPlugin::KAddressbookPlugin(Kontact::Core *_core, const char *, const QStringList & /*args*/ )
  : Kontact::Plugin(i18n("Contacts"), "kaddressbook", _core, _core, "kaddressbook"), 
    m_part(0)
{
  m_stub = 0L;
  setInstance(KAddressbookPluginFactory::instance());

  setXMLFile("kpkaddressbookplugin.rc");

  insertNewAction( new KAction( i18n( "New Contact" ), BarIcon( "contact" ),
			  0, this, SLOT( slotNewContact() ), actionCollection(), "new_contact" ));

}


KAddressbookPlugin::~KAddressbookPlugin()
{
}

KParts::Part* KAddressbookPlugin::part()
{
  if (!m_part)
  {
    m_part = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkaddressbookpart",
                                                             0, 0, // parentwidget,name
                                                             this, 0 ); // parent,name
    if (!m_part)
      return 0L;

    // 1) Register with dcop as "kaddressbook"  [maybe the part should do this]
    // 2) Create the stub that allows us to talk to the part
    m_stub = new KAddressBookIface_stub(dcopClient(), "kaddressbook", "KAddressBookIface");
  }
  return m_part;
}

QStringList KAddressbookPlugin::configModules() const
{
  QStringList modules;
  modules << "PIM/kabconfig.desktop" << "PIM/kabldapconfig.desktop";
  return modules;
}

KAboutData* KAddressbookPlugin::aboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", I18N_NOOP( "KAddressBook" ),
                                     "3.1", I18N_NOOP( "The KDE Address Book" ),
                                     KAboutData::License_BSD,
                                     I18N_NOOP( "(c) 1997-2002, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( "Don Sanders", I18N_NOOP( "Original author" ) );
  about->addAuthor( "Cornelius Schumacher",
                   I18N_NOOP( "Co-maintainer, libkabc port, CSV import/export" ),
                  "schumacher@kde.org" );
  about->addAuthor( "Mike Pilone", I18N_NOOP( "GUI and framework redesign" ),
                   "mpilone@slac.com" );
  about->addAuthor( "Greg Stern", I18N_NOOP( "DCOP interface" ) );
  about->addAuthor( "Mark Westcott", I18N_NOOP( "Contact pinning" ) );
  about->addAuthor( "Mischel Boyer de la Giroday", I18N_NOOP( "LDAP Lookup" ),
                    "michel@klaralvdalens-datakonsult.se" );
  about->addAuthor( "Steffen Hansen", I18N_NOOP( "LDAP Lookup" ),
                    "hansen@kde.org" );

  return about;
}

void KAddressbookPlugin::slotNewContact()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_stub );
  if ( m_stub )
      m_stub->newContact();
}

bool KAddressbookPlugin::createDCOPInterface( const QString& serviceType )
{
    if ( serviceType == "DCOP/AddressBook" )
    {
        Q_ASSERT( m_stub );
        return true;
    }
    return false;
}

QWidget *KAddressbookPlugin::createSummaryWidget( QWidget* parentWidget )
{
  return new KABSummaryWidget( parentWidget );
}
