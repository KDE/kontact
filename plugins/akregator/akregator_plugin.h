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

#if KDE_VERSION > KDE_MAKE_VERSION( 3, 3, 80 )
#include <libkdepim/part.h>
#else
namespace KPIM{
typedef KParts::Part Part;
}
#endif

class KAboutData;

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
	    
  protected:
    KPIM::Part *createPart();
    Akregator::aKregatorPartIface_stub *m_stub;
};

#endif
