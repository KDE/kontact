/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <kuniqueapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <qlabel.h>

#include "splash.h"
#include "core.h"

static const char *description =
    I18N_NOOP("A KDE Personal Information Manager");

static const char *version = "0.11";

/*
static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    { 0, 0, 0 }
};
*/


int main(int argc, char **argv)
{
    KAboutData about("kontact", I18N_NOOP("Kontact"), version, description,
                     KAboutData::License_GPL, "(C) 2001-2003 The Kontact developers", 0, "http://kontact.kde.org", "kde-pim@kde.org");
    about.addAuthor( "Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org" );
    about.addAuthor( "Daniel Molkentin", 0, "molkentin@kde.org" );
    about.addAuthor( "Don Sanders", 0, "sanders@kde.org" );
    about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
//    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication app;

    // show splash
    Splash *s = new Splash( 0, "splash" );
    s->show();

    // see if we are starting with session management
    if (app.isRestored())
        RESTORE(Core)
    else
    {
        // no session.. just start up normally
        Core *widget = new Core;
        widget->show();
    }

    // delete splash
    delete s;

    return app.exec();
}
// vim: ts=4 sw=4 et
