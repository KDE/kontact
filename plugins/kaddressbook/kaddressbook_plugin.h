/*
    This file is part of Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOK_PLUGIN_H
#define KADDRESSBOOK_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "kaddressbookiface_stub.h"
#include "plugin.h"
#include <uniqueapphandler.h>

class KAboutData;

class KABUniqueAppHandler : public Kontact::UniqueAppHandler
{
public:
    KABUniqueAppHandler( Kontact::Plugin* plugin ) : Kontact::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KAddressbookPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KAddressbookPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KAddressbookPlugin();

    virtual bool createDCOPInterface( const QString &serviceType );
    virtual bool isRunningStandalone();
    int weight() const { return 300; }

    bool canDecodeDrag( QMimeSource * );
    void processDropEvent( QDropEvent * );

    virtual QStringList configModules() const;

    virtual QStringList invisibleToolbarActions() const;

    virtual Kontact::Summary *createSummaryWidget( QWidget *parentWidget );

    virtual void configUpdated();

    KAddressBookIface_stub *interface();

  protected:
    KParts::ReadOnlyPart *createPart();
  private slots:
    void slotNewContact();

  private:
    KAddressBookIface_stub *mStub;
    Kontact::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
