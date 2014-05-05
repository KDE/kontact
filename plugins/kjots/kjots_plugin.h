/*
  This file is part of KJots.

  Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTS_PLUGIN_H
#define KJOTS_PLUGIN_H

#include "kjotswidgetinterface.h"

#include <KontactInterface/kontactinterface/UniqueAppHandler>

namespace KontactInterface {
  class Plugin;
}

class KJotsUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
  public:
    explicit KJotsUniqueAppHandler( KontactInterface::Plugin *plugin )
      : KontactInterface::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KJotsPlugin : public KontactInterface::Plugin
{
  Q_OBJECT

  public:
    KJotsPlugin( KontactInterface::Core *core, const QVariantList & );
    ~KJotsPlugin();
    int weight() const { return 550; }
    QStringList invisibleToolbarActions() const;
    bool isRunningStandalone() const;
    virtual bool queryClose() const;

    OrgKdeKJotsWidgetInterface *interface();

  private slots:
    void newPage();
    void newBook();

  protected:
    KParts::ReadOnlyPart *createPart();
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
    OrgKdeKJotsWidgetInterface *m_interface;
};

#endif

