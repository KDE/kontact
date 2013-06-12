/*
  This file is part of Kontact.

  Copyright (c) 2003 Zack Rusin <zack@kde.org>

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

#ifndef KNODE_PLUGIN_H
#define KNODE_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>

namespace KontactInterface {
  class Plugin;
}

class OrgKdeKnodeInterface;

class KNodeUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
  public:
    explicit KNodeUniqueAppHandler( KontactInterface::Plugin *plugin )
      : KontactInterface::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KNodePlugin : public KontactInterface::Plugin
{
  Q_OBJECT

  public:
    KNodePlugin( KontactInterface::Core *core, const QVariantList & );
    ~KNodePlugin();

    bool createDBUSInterface( const QString &serviceType );
    bool isRunningStandalone() const;
    QString tipFile() const;
    int weight() const { return 500; }

    QStringList invisibleToolbarActions() const;

  protected:
    virtual KParts::ReadOnlyPart *createPart();

  protected slots:
    void slotPostArticle();

  private:
    OrgKdeKnodeInterface *m_interface;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
