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
  : Kontact::Plugin(i18n("Test"), "test", _core, _core, name), m_part(0)
{
  setInstance(TestPluginFactory::instance());

  insertNewAction(new KAction("Test", 0, this, SLOT(slotTestMenu()), actionCollection(), "edit_test"));

  setXMLFile("kptestplugin.rc");
}

TestPlugin::~TestPlugin()
{
}

void TestPlugin::slotTestMenu()
{
  core()->statusBar()->message("Test menu activated");
}

KParts::Part* TestPlugin::part()
{
  if (!m_part)
  {
    m_part = new TestPart(this, "testpart");
  }

  return m_part;
}

#include "test_plugin.moc"
