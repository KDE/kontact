#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>

#include "kpcore.h"


#include "test_plugin.h"
#include "test_part.h"

typedef KGenericFactory< TestPlugin, Kaplan::Core > TestPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkptestplugin, TestPluginFactory( "kptestplugin" ) );

TestPlugin::TestPlugin(Kaplan::Core *_core, const char *name, const QStringList &)
  : Kaplan::Plugin(_core, _core, name), m_part(0)
{
  setInstance(TestPluginFactory::instance());

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

#include "test_plugin.moc"
