#ifndef __KORGANIZER_PLUGIN_H__
#define __KORGANIZER_PLUGIN_H__


#include <kparts/part.h>


#include "kpplugin.h"


class KOrganizerPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KOrganizerPlugin(Kaplan::Core *core, const char *name, const QStringList &);
  ~KOrganizerPlugin();

  
private slots:
  
  void slotShowPlugin();


private:

  KParts::ReadOnlyPart *m_part;

};


#endif
