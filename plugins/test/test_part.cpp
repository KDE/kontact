#include "test_part.h"
#include "kaddressbookiface_stub.h"

#include <qtextedit.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>


TestPart::TestPart(QObject *parent, const char *name) // ## parentWidget
  : KParts::ReadOnlyPart(parent, name)
{
  setInstance( new KInstance("testpart") ); // ## memleak
  m_edit = new QTextEdit;
  setWidget(m_edit);
  setXMLFile("testpartui.rc");
  new KAction( i18n("new contact (test)"), 0, this, SLOT( newContact() ), actionCollection(), "test_deleteevent" );

  // TODO move generic interfaces to kdelibs
  // The dynamic way (dcopclient->call) sucks - the code would have to be rewritten
  // Better share the header file, even if it means copying it between modules..... :}

}

void TestPart::newContact()
{
  // TODO HostExtension for creating DCOP services, implemented in kaplan/kontact
  // to load the right plugin+part
  // TODO put the dcopobject id into the service's .desktop file
  m_kab_stub = new KAddressBookIface_stub(kapp->dcopClient(), "kaddressbook", "KAddressBookIface");
  if ( !m_kab_stub->ok() ) {
    kdDebug() << "Communication problem" << endl;
    delete m_kab_stub;
    m_kab_stub = 0L;
    return;
  }

  m_kab_stub->newContact();
}

bool TestPart::openFile()
{
  m_edit->setText(m_file);
  return true;
}


#include "test_part.moc"
