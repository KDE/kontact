#ifndef KORG_UNIQUEAPP_H
#define KORG_UNIQUEAPP_H

#include <uniqueapphandler.h>

class KOrganizerUniqueAppHandler : public Kontact::UniqueAppHandler
{
public:
    KOrganizerUniqueAppHandler( Kontact::Plugin* plugin ) : Kontact::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};


#endif /* KORG_UNIQUEAPP_H */

