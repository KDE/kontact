/*
  This file is part of KDE Kontact.

  Copyright (c) 2004 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KONTACT_JOURNALPLUGIN_H
#define KONTACT_JOURNALPLUGIN_H

#include <kontactinterfaces/plugin.h>

namespace Kontact {
  class UniqueAppWatcher;
}

class OrgKdeKorganizerCalendarInterface;

class JournalPlugin : public Kontact::Plugin
{
  Q_OBJECT
  public:
    JournalPlugin( Kontact::Core *core, const QVariantList & );
    ~JournalPlugin();

    virtual bool createDBUSInterface( const QString &serviceType );
    virtual bool isRunningStandalone();
    int weight() const { return 525; }

    virtual QStringList invisibleToolbarActions() const;

    void select();

    OrgKdeKorganizerCalendarInterface *interface();

  protected:
    KParts::ReadOnlyPart *createPart();

  private slots:
    void slotNewJournal();
    void slotSyncJournal();

  private:
    OrgKdeKorganizerCalendarInterface *mIface;
    Kontact::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
