#ifndef __KADDRESSBOOK_PLUGIN_H__
#define __KADDRESSBOOK_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"


class KAddressbookPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KAddressbookPlugin(Kaplan::Core *core, const char *name, const QStringList & /*args*/);
  ~KAddressbookPlugin();

  
private slots:
  
  void slotShowPlugin();


private:

  KParts::ReadOnlyPart *m_part;

};


#endif
