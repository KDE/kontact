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

  You should have received a copy of the GNU General Public Licensea along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface {
  class Plugin;
}

class OrgKdeAkregatorPartInterface;

class AkregatorUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
  public:
    AkregatorUniqueAppHandler( KontactInterface::Plugin *plugin )
      : KontactInterface::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class AkregatorPlugin : public KontactInterface::Plugin
{
  Q_OBJECT

  public:
    AkregatorPlugin( KontactInterface::Core *core, const QVariantList & );
    ~AkregatorPlugin();

    virtual QString tipFile() const;
    int weight() const { return 475; }

    OrgKdeAkregatorPartInterface *interface();

    virtual QStringList configModules() const;
    virtual QStringList invisibleToolbarActions() const;
    virtual bool isRunningStandalone() const;
    virtual void readProperties( const KConfigGroup &config );
    virtual void saveProperties( KConfigGroup &config );

  private slots:
    void showPart();
    void addFeed();

  protected:
    KParts::ReadOnlyPart *createPart();
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
    OrgKdeAkregatorPartInterface *m_interface;
};

#endif
