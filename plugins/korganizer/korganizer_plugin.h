#ifndef __KORGANIZER_PLUGIN_H__
#define __KORGANIZER_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
//#include "korganizeriface_stub.h"

class KOrganizerPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KOrganizerPlugin(Kaplan::Core *core, const char *name, const QStringList &);
  ~KOrganizerPlugin();

  virtual bool createDCOPInterface( const QString& serviceType );

protected:
  void loadPart();

private slots:

  void slotShowPart();
  void slotNewAppointment();

private:

  KParts::ReadOnlyPart *m_part;
  //KOrganizerIface_stub *m_iface;
};


#endif
