#ifndef __KADDRESSBOOK_PLUGIN_H__
#define __KADDRESSBOOK_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
#include "kaddressbookiface_stub.h"


class KAddressbookPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KAddressbookPlugin(Kaplan::Core *core, const char *name, const QStringList & /*args*/);
  ~KAddressbookPlugin();

  
private slots:
  
  void slotShowPlugin();
  void slotNewContact();

private:
  void loadPart();
  KAddressBookIface_stub *m_stub;
  KParts::ReadOnlyPart *m_part;

};


#endif
