/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

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
