#ifndef __TEST_PLUGIN_H__
#define __TEST_PLUGIN_H__


#include "kpplugin.h"


class TestPart;


class TestPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  TestPlugin(Kaplan::Core *core, const char *name, const QStringList &);
  ~TestPlugin();

  
private slots:
  
  void slotTestMenu();
  void slotShowNotes();


private:

  TestPart *m_part;

};


#endif
