#include <kmessagebox.h>
#include <kaction.h>


#include "kpcore.h"
#include "kpfactory.h"


#include "test_plugin.h"
#include "test_part.h"


TestPlugin::TestPlugin(Kaplan::Core *_core, QObject *parent, const char *name)
  : Kaplan::Plugin(_core, parent, name), m_part(0)
{
  setInstance(Kaplan::FactoryImpl<TestPlugin>::instance("kptestplugin"));

  new KAction("Test", 0, this, SLOT(slotTestMenu()), actionCollection(), "edit_test");
  
  setXMLFile("kptestplugin.rc");

  core()->addMainEntry("Mail", "kmail", this, SLOT(slotTestMenu()));
  core()->addMainEntry("News", "knode", this, SLOT(slotTestMenu()));
  core()->addMainEntry("Notes", "knotes", this, SLOT(slotShowNotes()));
}


TestPlugin::~TestPlugin()
{
}


void TestPlugin::slotTestMenu()
{
  KMessageBox::information(0, "Test menu activated");
}


void TestPlugin::slotShowNotes()
{
  if (!m_part)
  {
    m_part = new TestPart(this, "notes");
    core()->addPart(m_part);
  }
 
  core()->showView(m_part->widget()); 
}


extern "C"
{
         
  void *init_libkptestplugin()
  {
    return new Kaplan::FactoryImpl<TestPlugin>();
  }
          
};


#include "test_plugin.moc"
