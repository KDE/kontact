#ifndef __KMAIL_PLUGIN_H__
#define __KMAIL_PLUGIN_H__


#include <kparts/part.h>


#include "plugin.h"
#include "kmailIface_stub.h"


class KMailPlugin : public Kontact::Plugin
{
  Q_OBJECT

public:

  KMailPlugin(Kontact::Core *core, const char *name, const QStringList & /*args*/);
  ~KMailPlugin();

  virtual KParts::Part* part();
  virtual bool createDCOPInterface( const QString& serviceType );
  virtual QWidget* createSummaryWidget( QWidget *parent );

protected slots:
  void slotNewMail();

private:
  KParts::ReadOnlyPart *m_part;
  KMailIface_stub *m_stub;
};


#endif
