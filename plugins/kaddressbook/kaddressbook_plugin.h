#ifndef KADDRESSBOOK_PLUGIN_H
#define KADDRESSBOOK_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "kaddressbookiface_stub.h"
#include "plugin.h"

class KAboutData;
class KABSummaryWidget;

class KAddressbookPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KAddressbookPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KAddressbookPlugin();

    QString identifier() const { return "contacts"; }
    QString title() const { return i18n( "Contacts" ); }
    QString icon() const { return "kaddressbook"; }

    virtual bool createDCOPInterface( const QString& serviceType );
    virtual QStringList configModules() const;
    KAboutData* aboutData();
    KParts::Part* part();

    virtual QWidget *createSummaryWidget( QWidget* parentWidget );
  
  private slots:
    void slotNewContact();

  private:
    KAddressBookIface_stub *m_stub;
    KParts::ReadOnlyPart *m_part;
};

#endif
