#ifndef KMAIL_PLUGIN_H
#define KMAIL_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "kmailIface_stub.h"
#include "plugin.h"

class KMailPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KMailPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KMailPlugin();

    QString identifier() const { return "mail"; }
    QString title() const { return i18n( "Mail" ); }
    QString icon() const { return "kmail"; }

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
