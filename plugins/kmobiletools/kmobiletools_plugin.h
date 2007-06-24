/***************************************************************************
   Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
   Copyright (C) 2007 Marco Gulino <marco@kmobiletools.org>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/


#ifndef KMOBILETOOLS_PLUGIN_H
#define KMOBILETOOLS_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include <plugin.h>
#include <uniqueapphandler.h>

class OrgKdeKmobiletoolsMainInterface;


class KMobileToolsPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KMobileToolsPlugin( Kontact::Core *core, const QStringList & );
    ~KMobileToolsPlugin();

    int weight() const { return 700; }
    bool isRunningStandalone();

  protected:
    KParts::ReadOnlyPart *createPart();
    bool partLoaded;
    OrgKdeKmobiletoolsMainInterface *m_interface;
    public slots:
        void slotNewSMS();
};

#endif
