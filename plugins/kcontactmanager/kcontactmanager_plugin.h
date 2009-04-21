/*
    This file is part of KContactManager.

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

#ifndef KCONTACTMANAGER_PLUGIN_H
#define KCONTACTMANAGER_PLUGIN_H


#include <kontactinterfaces/plugin.h>
#include <kontactinterfaces/uniqueapphandler.h>

#include <kparts/part.h>

class KContactManagerUniqueAppHandler : public Kontact::UniqueAppHandler
{
  public:
    KContactManagerUniqueAppHandler( Kontact::Plugin *plugin ) : Kontact::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KContactManagerPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KContactManagerPlugin( Kontact::Core *core, const QVariantList & );
    ~KContactManagerPlugin();
    int weight() const { return 550; }

  protected:
    KParts::ReadOnlyPart *createPart();
};

#endif

