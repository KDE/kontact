/*
  This file is part of the KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KONTACT_SIDEPANEBASE_H
#define KONTACT_SIDEPANEBASE_H

#include <KVBox>

namespace KontactInterface {
  class Core;
  class Plugin;
}


namespace Kontact {

class SidePaneBase : public KVBox
{
  Q_OBJECT

  public:
    SidePaneBase( KontactInterface::Core *core, QWidget *parent );
    virtual ~SidePaneBase();

    virtual void setCurrentPlugin( const QString & ) = 0;

  signals:
    void pluginSelected( KontactInterface::Plugin * );

  public slots:
    /**
      This method is called by the core whenever the count
      of plugins has changed.
     */
    virtual void updatePlugins() = 0;

  protected:
    KontactInterface::Core *core() const;

  private:
    KontactInterface::Core *mCore;
};

}

#endif
