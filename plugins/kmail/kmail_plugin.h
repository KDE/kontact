/*
    This file is part of Kontact.
    Copyright (c) 2003 Kontact Developer

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

#ifndef KMAIL_PLUGIN_H
#define KMAIL_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include "kmailIface_stub.h"
#include "plugin.h"
#include "summary.h"

class KMailPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KMailPlugin( Kontact::Core *core, const char *name, const QStringList& );
    ~KMailPlugin();

    virtual bool isRunningStandalone();
    virtual bool createDCOPInterface( const QString& serviceType );
    virtual Kontact::Summary *createSummaryWidget( QWidget *parent );
    virtual QString tipFile() const;

  protected:
    virtual KParts::Part* createPart();

  protected slots:
    void slotNewMail();

  private:
    KMailIface_stub *mStub;
};

#endif
