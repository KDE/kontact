#ifndef __KMAIL_PLUGIN_H__
#define __KMAIL_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"
#include "kmailpartIface_stub.h"


class KMailPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KMailPlugin(Kaplan::Core *core, const char *name, const QStringList & /*args*/);
  ~KMailPlugin();


private slots:

  void slotShowPlugin();

private:
  void loadPart();
  KMailPartIface_stub *m_stub;
  KParts::ReadOnlyPart *m_part;
};


#endif
