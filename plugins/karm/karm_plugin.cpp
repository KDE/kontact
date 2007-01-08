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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kicon.h>
#include <kactioncollection.h>
#include "core.h"
#include "plugin.h"

#include "karm_plugin.h"
#include "karminterface.h"

typedef KGenericFactory<KarmPlugin, Kontact::Core> KarmPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_karm,
                            KarmPluginFactory( "kontact_karm" ) )

KarmPlugin::KarmPlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "KArm" )
{
  setInstance( KarmPluginFactory::instance() );

    KAction *action  = new KAction(KIcon("karm"), i18n("New Task"), this);
    actionCollection()->addAction("new_task", action );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_W));
  connect(action, SIGNAL(triggered(bool)), SLOT( newTask() ));
  insertNewAction(action);
}

KarmPlugin::~KarmPlugin()
{
}

QString KarmPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "karm/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart* KarmPlugin::createPart()
{
  KParts::ReadOnlyPart * part = loadPart();
  if ( !part ) return 0;

  mInterface = new OrgKdeKarmKarmInterface( "org.kde.karm", "/Karm", QDBusConnection::sessionBus() );

  return part;
}

void KarmPlugin::newTask()
{
  kDebug() << "Entering newTask" << endl;
  mInterface->addTask("New Task");
}

#include "karm_plugin.moc"
