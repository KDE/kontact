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


private slots:

  void slotTestMenu();
  void slotShowPart();


private:
  void loadPart();

  TestPart *m_part;

};


#endif
