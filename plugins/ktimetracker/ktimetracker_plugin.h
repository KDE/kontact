/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  adapted for karm 2005 by Thorsten Staerk <kde@staerk.de>

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

#ifndef KTIMETRACKER_PLUGIN_H
#define KTIMETRACKER_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface {
  class Plugin;
}

class OrgKdeKtimetrackerKtimetrackerInterface;

class KtimetrackerUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
  public:
    explicit KtimetrackerUniqueAppHandler( KontactInterface::Plugin *plugin )
      : KontactInterface::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class ktimetrackerplugin : public KontactInterface::Plugin
{
  Q_OBJECT

  public:
    ktimetrackerplugin( KontactInterface::Core *core, const QVariantList & );
    ~ktimetrackerplugin();
    int weight() const { return 700; }
    virtual QStringList invisibleToolbarActions() const;
    virtual bool isRunningStandalone() const;

    OrgKdeKtimetrackerKtimetrackerInterface *interface();

    virtual QStringList configModules() const;

  private slots:
    void newTask();

  protected:
    KParts::ReadOnlyPart *createPart();

  protected Q_SLOTS:
    void taskViewCustomContextMenuRequested( const QPoint & );

  private:
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
    OrgKdeKtimetrackerKtimetrackerInterface *mInterface;
};

#endif
