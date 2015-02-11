/*
  This file is part of KAddressBook Kontact Plugin.

  Copyright (c) 2009-2015 Laurent Montel <montel@kde.org>

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

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface
{
class Plugin;
}

class KAddressBookUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
public:
    explicit KAddressBookUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin) {}
    void loadCommandLineOptions() Q_DECL_OVERRIDE;
    int newInstance() Q_DECL_OVERRIDE;
};

class KAddressBookPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    KAddressBookPlugin(KontactInterface::Core *core, const QVariantList &);
    ~KAddressBookPlugin();

    QString tipFile() const Q_DECL_OVERRIDE;
    bool isRunningStandalone() const Q_DECL_OVERRIDE;
    int weight() const Q_DECL_OVERRIDE
    {
        return 300;
    }

    QStringList invisibleToolbarActions() const Q_DECL_OVERRIDE;
    void shortcutChanged() Q_DECL_OVERRIDE;

protected:
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotNewContact();
    void slotNewContactGroup();
    void slotSyncContacts();

private:
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;

};

#endif

