#ifndef __KORGANIZER_PLUGIN_H__
#define __KORGANIZER_PLUGIN_H__


#include <kparts/part.h>


#include "plugin.h"
#include "korganizeriface_stub.h"

class KOrganizerPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:

  KOrganizerPlugin(Kontact::Core *core, const char *name, const QStringList &);
  ~KOrganizerPlugin();

  virtual bool createDCOPInterface( const QString& serviceType );

protected:
  KParts::ReadOnlyPart* part();

private slots:
  void slotNewEvent();
  void slotNewTodo();

private:

  KParts::ReadOnlyPart *m_part;
  KOrganizerIface_stub *m_iface;
};


#endif
