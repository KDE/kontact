#ifndef __TEST_PLUGIN_H__
#define __TEST_PLUGIN_H__


#include "plugin.h"


class TestPart;


class TestPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:

  TestPlugin(Kontact::Core *core, const char *name, const QStringList &);
  ~TestPlugin();

protected:
  KParts::Part* createPart();
  
private slots:

  void slotTestMenu();
};


#endif
