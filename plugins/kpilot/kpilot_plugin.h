/*
   This file is part of Kontact.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>
	 Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KPilot_PLUGIN_H
#define KPilot_PLUGIN_H

#include "plugin.h"
#include "pilotDaemonDCOP_stub.h"

class SummaryWidget;

class KPilotPlugin : public Kontact::Plugin
{
  public:
    KPilotPlugin( Kontact::Core *core, const char *name, const QStringList& );
    KPilotPlugin();

    virtual Kontact::Summary *createSummaryWidget( QWidget *parentWidget );

    virtual bool showInSideBar() const { return false; }
//    virtual QStringList configModules() const;

    const KAboutData *aboutData();

  protected:
    virtual KParts::ReadOnlyPart *createPart() { return 0; }
  private:
    KAboutData *mAboutData;
};

#endif
