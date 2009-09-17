/*
    This file is part of KAddressBook Kontact Plugin.

    Copyright (c) 2009 Laurent Montel <montel@kde.org>

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
*/

#ifndef KADDRESSBOOK_PLUGIN_H
#define KADDRESSBOOK_PLUGIN_H

#include <kontactinterface/plugin.h>
#include <kontactinterface/uniqueapphandler.h>

#include <kparts/part.h>

class KAddressBookUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
  public:
    KAddressBookUniqueAppHandler( KontactInterface::Plugin *plugin ) : KontactInterface::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KAddressBookPlugin : public KontactInterface::Plugin
{
  Q_OBJECT

  public:
    KAddressBookPlugin( KontactInterface::Core *core, const QVariantList & );
    ~KAddressBookPlugin();

    virtual QString tipFile() const;
    virtual bool isRunningStandalone();
    int weight() const { return 300; }

    virtual QStringList invisibleToolbarActions() const;

  protected:
    KParts::ReadOnlyPart *createPart();

  private Q_SLOTS:
    void slotNewContact();
    void slotNewContactGroup();
    void slotSyncContacts();

  private:
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif

