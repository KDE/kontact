#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>

#include "core.h"

#include "test_plugin.h"
#include "test_part.h"

typedef KGenericFactory< TestPlugin, Kontact::Core > TestPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkptestplugin, TestPluginFactory( "kptestplugin" ) );

TestPlugin::TestPlugin(Kontact::Core *_core, const char *name, const QStringList &)
  : Kontact::Plugin(i18n("Test"), "test", _core, _core, name)
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

KParts::Part* TestPlugin::createPart()
{
  return new TestPart(this, "testpart");
}

#include "test_plugin.moc"
