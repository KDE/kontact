#ifndef KORGANIZER_PLUGIN_H
#define KORGANIZER_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "korganizeriface_stub.h"
#include "plugin.h"

class KOrganizerPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KOrganizerPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KOrganizerPlugin();

    QString identifier() const { return "calendar"; }
    QString title() const { return i18n( "Calendar" ); }
    QString icon() const { return "korganizer"; }

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
