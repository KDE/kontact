/*
   This file is part of Kontact

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

// $Id$

#ifndef KP_CORE_H
#define KP_CORE_H


#include <kparts/mainwindow.h>
#include <kparts/part.h>

class KAction;

namespace Kontact
{

    class Plugin;
    /**
     * This class is now purely private to Kontact and not visible for the plugins
     **/
    class Core : public KParts::MainWindow
    {
        protected:

            Core(QWidget *parentWidget = 0, const char *name = 0);
        public:
            virtual ~Core();

            virtual void showPart(KParts::Part *part, Kontact::Plugin *plugin) = 0;

			virtual QPtrList<Kontact::Plugin> pluginList() = 0;
    };


};

#endif
// vim: ts=4 sw=4 et

