/*
  This file is part of the KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KONTACT_SIDEPANEBASE_H
#define KONTACT_SIDEPANEBASE_H

#include <qvbox.h>

namespace KParts { class Part; }

namespace Kontact
{

class Plugin;

class SidePaneBase : public QVBox
{
    Q_OBJECT
  public:
    SidePaneBase( QWidget *parent, const char *name = 0 );
    virtual ~SidePaneBase();

    virtual QString currentPluginName() const = 0;
  
  signals:
    void showPart( Kontact::Plugin * );

  public slots:
    /**
      Adds a new entry to the sidepane.
    */
    virtual void addEntry( Kontact::Plugin *plugin ) = 0;

    virtual void selectPlugin( const QString &name ) = 0;
};

}

#endif

// vim: ts=2 sw=2 et
