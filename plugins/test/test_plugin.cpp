#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>

#include "kpcore.h"


#include "test_plugin.h"
#include "test_part.h"

typedef KGenericFactory< TestPlugin, Kontact::Core > TestPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkptestplugin, TestPluginFactory( "kptestplugin" ) );

TestPlugin::TestPlugin(Kontact::Core *_core, const char *name, const QStringList &)
  : Kontact::Plugin(_core, _core, name), m_part(0)
{
  setInstance(TestPluginFactory::instance());

  new KAction("Test", 0, this, SLOT(slotTestMenu()), actionCollection(), "edit_test");

  setXMLFile("kptestplugin.rc");

  core()->addMainEntry("Test", "configure", this, SLOT(slotShowPart()));
}


TestPlugin::~TestPlugin()
{
}


void TestPlugin::slotTestMenu()
{
  core()->statusBar()->message("Test menu activated");
}

void TestPlugin::loadPart()
{
  if (!m_part)
  {
    m_part = new TestPart(this, "testpart");
    core()->addPart(m_part);
  }
}

void TestPlugin::slotShowPart()
{
  loadPart();
  core()->showPart(m_part);
}

#include "test_plugin.moc"
