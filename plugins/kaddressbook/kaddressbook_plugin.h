#ifndef __KADDRESSBOOK_PLUGIN_H__
#define __KADDRESSBOOK_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
#include "kaddressbookiface_stub.h"


class KAboutData;

class KAddressbookPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:
  KAddressbookPlugin(Kontact::Core *core, const char *name, const QStringList & /*args*/);
  ~KAddressbookPlugin();

  virtual bool createDCOPInterface( const QString& serviceType );
  virtual QStringList configModules() const;
  KAboutData* aboutData();
  KParts::Part* part();

  virtual QWidget *summaryWidget();
  
private slots:
  void slotNewContact();

private:
  KAddressBookIface_stub *m_stub;
  KParts::ReadOnlyPart *m_part;

};


#endif
