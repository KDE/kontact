#ifndef __TEST_PLUGIN_H__
#define __TEST_PLUGIN_H__


#include "kpplugin.h"


class TestPart;


class TestPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:

  TestPlugin(Kontact::Core *core, const char *name, const QStringList &);
  ~TestPlugin();

  KParts::Part* part();
  
private slots:

  void slotTestMenu();

private:

  TestPart *m_part;

};


#endif
