#ifndef __KADDRESSBOOK_PLUGIN_H__
#define __KADDRESSBOOK_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"


class KAddressbookPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KAddressbookPlugin(Kaplan::Core *core, QObject *parent=0, const char *name=0);
  ~KAddressbookPlugin();

  
private slots:
  
  void slotShowPlugin();


private:

  KParts::ReadOnlyPart *m_part;

};


#endif
