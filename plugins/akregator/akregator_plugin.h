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

#include <libkdepim/part.h>
#include <kontact/plugin.h>
#include "akregator_partiface_stub.h"
#include <uniqueapphandler.h>

typedef KPIM::Part MyBasePart;

class KAboutData;

class AkregatorUniqueAppHandler : public Kontact::UniqueAppHandler
{
    public:
        AkregatorUniqueAppHandler( Kontact::Plugin* plugin ) : Kontact::UniqueAppHandler( plugin ) {}
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

    int weight() const { return 700; }

    Akregator::aKregatorPartIface_stub *interface();

    virtual QStringList configModules() const;
    virtual QStringList invisibleToolbarActions() const;
    virtual bool isRunningStandalone();
    
  private slots:
    void showPart();

  protected:
    MyBasePart *createPart();
    Akregator::aKregatorPartIface_stub *m_stub;
    Kontact::UniqueAppWatcher *m_uniqueAppWatcher;
};

#endif
