/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
                 2004 Michael Brade <brade@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef KNOTES_PLUGIN_H
#define KNOTES_PLUGIN_H

#include <klocale.h>

#include "plugin.h"

class KNotesPart;
class SummaryWidget;


class KNotesPlugin : public Kontact::Plugin
{
    Q_OBJECT
public:
    KNotesPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KNotesPlugin();

    virtual Kontact::Summary *createSummaryWidget( QWidget *parentWidget );

    int weight() const { return 600; }

    const KAboutData *aboutData();

protected:
    KParts::ReadOnlyPart* createPart();

private slots:
    void slotNewNote();

private:
    KAboutData *mAboutData;
};

#endif
