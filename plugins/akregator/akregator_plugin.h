/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include <kparts/part.h>
#include <plugin.h>
#include "akregator_partiface_stub.h"
#include <uniqueapphandler.h>

class KAboutData;

namespace Akregator {

typedef KParts::ReadOnlyPart MyBasePart;
      
class UniqueAppHandler : public Kontact::UniqueAppHandler
{
    public:
        UniqueAppHandler( Kontact::Plugin* plugin ) : Kontact::UniqueAppHandler( plugin ) {}
        virtual void loadCommandLineOptions();
        virtual int newInstance();
};


class Plugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    Plugin( Kontact::Core *core, const char *name,
                       const QStringList & );
    ~Plugin();

    int weight() const { return 700; }

    AkregatorPartIface_stub *interface();

    virtual QStringList configModules() const;
    virtual QStringList invisibleToolbarActions() const;
    virtual bool isRunningStandalone();
    
  private slots:
    void showPart();

  protected:
    MyBasePart *createPart();
    AkregatorPartIface_stub *m_stub;
    Kontact::UniqueAppWatcher *m_uniqueAppWatcher;
};

} // namespace Akregator
#endif
