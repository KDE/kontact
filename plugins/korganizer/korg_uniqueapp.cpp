#include "korg_uniqueapp.h"
#include <kdebug.h>
#include "../../korganizer/korganizer_options.h"

void KOrganizerUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( korganizer_options );
}

int KOrganizerUniqueAppHandler::newInstance()
{
    kdDebug() << k_funcinfo << endl;
    // TODO handle command line options
    return Kontact::UniqueAppHandler::newInstance();
}
