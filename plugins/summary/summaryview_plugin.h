/*
   This file is part of KDE Kontact.

   Copyright (C) 2003 Sven L�ppken <sven@kde.org>

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

#ifndef SUMMARYVIEW_PLUGIN_H
#define SUMMARYVIEW_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "plugin.h"

class SummaryView : public Kontact::Plugin
{
  Q_OBJECT

  public:
    SummaryView( Kontact::Core *core, const char *name, const QStringList& );
	  ~SummaryView();

    int weight() const { return 100; }

    const KAboutData *aboutData();

  protected:
    virtual KParts::ReadOnlyPart* createPart();

  private:
    KAboutData *mAboutData;
};

#endif
