#include "test_part.h"
#include "kaddressbookiface_stub.h"

#include <qtextedit.h>
#include <qcombobox.h>

#include "sidebarextension.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdcopservicestarter.h>
#include <ktrader.h>
#include <kservice.h>


TestPart::TestPart(QObject *parent, const char *name) // ## parentWidget
  : KParts::ReadOnlyPart(parent, name)
{
  setInstance( new KInstance("testpart") ); // ## memleak
  m_edit = new QTextEdit;
  setWidget(m_edit);
  setXMLFile("testpartui.rc");
  new KAction( "new contact (test)", 0, this, SLOT( newContact() ), actionCollection(), "test_deleteevent" );
  m_kab_stub = 0L;

  new KParts::SideBarExtension(new QComboBox(this), this, "sbe");
  
  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString&)),
           this, SLOT( unregisteredFromDCOP( const QCString& )) );
}

TestPart::~TestPart()
{
  kapp->dcopClient()->setNotifications( false );
  delete m_kab_stub;
}

void TestPart::newContact()
{
  if ( !connectToAddressBook() )
    return;

  kdDebug() << "Calling newContact" << endl;
  m_kab_stub->newContact();

  // If critical call: test that it worked ok
  if ( !m_kab_stub->ok() ) {
    kdDebug() << "Communication problem - ERROR" << endl;
    // TODO handle the error
  }
}

bool TestPart::connectToAddressBook()
{
  if ( !m_kab_stub )
  {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->findServiceFor( "DCOP/AddressBook", QString::null, QString::null, &error, &dcopService );
    if ( result != 0 ) {
      // You might want to show "error" (if not empty) here, using e.g. KMessageBox
      return false;
    }
    // TODO document the required named for the dcop interfaces e.g. "CalendarIface".
    QCString dcopObjectId = "KAddressBookIface";
    m_kab_stub = new KAddressBookIface_stub(kapp->dcopClient(), dcopService, dcopObjectId);
  }
  return m_kab_stub != 0L;
}

void TestPart::unregisteredFromDCOP( const QCString& appId )
{
  if ( m_kab_stub && m_kab_stub->app() == appId )
  {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete m_kab_stub;
    m_kab_stub = 0L;
  }
}

bool TestPart::openFile()
{
  m_edit->setText(m_file);
  return true;
}

#include "test_part.moc"
