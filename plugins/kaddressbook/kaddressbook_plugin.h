#ifndef __KADDRESSBOOK_PLUGIN_H__
#define __KADDRESSBOOK_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
#include "kaddressbookiface_stub.h"


class KAboutData;

class KAddressbookPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KAddressbookPlugin(Kaplan::Core *core, const char *name, const QStringList & /*args*/);
  ~KAddressbookPlugin();

  virtual bool createDCOPInterface( const QString& serviceType );

  virtual QStringList configModules() const;

  KAboutData* aboutData();
  
private slots:

  void slotShowPart();
  void slotNewContact();

private:
  void loadPart();
  KAddressBookIface_stub *m_stub;
  KParts::ReadOnlyPart *m_part;

};


#endif
