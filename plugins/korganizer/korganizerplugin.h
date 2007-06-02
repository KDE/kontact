/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KORGANIZER_PLUGIN_H
#define KORGANIZER_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "kcalendariface_stub.h"
#include "plugin.h"
#include "uniqueapphandler.h"

class KOrganizerPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KOrganizerPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KOrganizerPlugin();

    virtual bool createDCOPInterface( const QString& serviceType );
    virtual bool isRunningStandalone();
    int weight() const { return 400; }

    bool canDecodeDrag( QMimeSource * );
    void processDropEvent( QDropEvent * );

    virtual Kontact::Summary *createSummaryWidget( QWidget *parent );

    virtual QString tipFile() const;
    virtual QStringList invisibleToolbarActions() const;

    void select();

    KCalendarIface_stub *interface();


    void loadProfile( const QString& path );
    void saveToProfile( const QString& path ) const;

  protected:
    KParts::ReadOnlyPart *createPart();

  private slots:
    void slotNewEvent();
    void slotSyncEvents();

  private:
    KCalendarIface_stub *mIface;
    Kontact::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
