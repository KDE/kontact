/*
   This file is part of Kontact

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

// $Id$

#include <assert.h>

#include <dcopclient.h>

#include "kpcore.h"
#include "kpplugin.h"

using namespace Kontact;

class Plugin::Private
{
public:
    Kontact::Core *core;
    DCOPClient *dcopClient;
    QCString name;
    QString pluginName;
    QString icon;
    QPtrList<KAction> *newActions;
};


Plugin::Plugin(const QString& pluginName, const QString& icon, Kontact::Core *core,
                QObject *parent, const char *name)
    : QObject(parent, name)
{
    d = new Kontact::Plugin::Private;
    d->name = name;
    d->core = core;
    d->icon = icon;
    d->pluginName = pluginName;
    d->dcopClient = 0L;
    d->newActions = new QPtrList<KAction>; 
}


Plugin::~Plugin()
{
    delete d->dcopClient;
    delete d;
}

QString Plugin::pluginName() const
{
    return d->pluginName;
}

QString Plugin::icon() const
{
    return d->icon;
}


DCOPClient* Plugin::dcopClient() const
{
    if (!d->dcopClient)
    {
        d->dcopClient = new DCOPClient();
        d->dcopClient->registerAs(d->name, false);
    }

    return d->dcopClient;
}

void Plugin::insertNewAction(KAction *action)
{
    d->newActions->append(action);
}

QPtrList<KAction>* Plugin::newActions() const
{
    return d->newActions;
}

// Protected
Kontact::Core* Plugin::core() const
{
    return d->core;
}

#include "kpplugin.moc"

// vim: ts=4 sw=4 et
