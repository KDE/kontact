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

#include <kontact/plugin.h>


#include "akregator_partiface_stub.h"
#include "uniqueapphandler.h"

#include "config.h"
#ifdef HAVE_KPIMPART
#include <libkdepim/part.h>
typedef KPIM::Part MyBasePart;
#else
typedef KParts::Part MyBasePart;
#endif

class KAboutData;

class AkregatorUniqueAppHandler : public Akregator::UniqueAppHandler
{
    public:
        AkregatorUniqueAppHandler( Kontact::Plugin* plugin ) : Akregator::UniqueAppHandler( plugin ) {}
        virtual void loadCommandLineOptions();
        virtual int newInstance();
};


class aKregatorPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    aKregatorPlugin( Kontact::Core *core, const char *name,
                       const QStringList & );
    ~aKregatorPlugin();

    int weight() const { return 100; }

    Akregator::aKregatorPartIface_stub *interface();

    virtual QStringList configModules() const;
    virtual QStringList invisibleToolbarActions() const;
    virtual bool isRunningStandalone();
    
  protected:
    MyBasePart *createPart();
    Akregator::aKregatorPartIface_stub *m_stub;
    Akregator::UniqueAppWatcher *m_uniqueAppWatcher;
};

#endif
