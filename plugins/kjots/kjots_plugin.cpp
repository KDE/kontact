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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kparts/componentfactory.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kicon.h>
#include <QWidget>
#include <kontactinterfaces/core.h>
#include <kontactinterfaces/plugin.h>

#include "kjots_plugin.h"

#include "kjotspart.h"


EXPORT_KONTACT_PLUGIN(KJotsPlugin, kjots)

KJotsPlugin::KJotsPlugin( Kontact::Core *core, const QVariantList& )
  : Kontact::Plugin( core, core, "kjots" )//, m_interface(0)
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action = new KAction(KIcon("document-new"), i18n("New KJots Page"), this);
  actionCollection()->addAction("new_kjots_page", action );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_J));
  connect(action, SIGNAL(triggered(bool)), SLOT( newPage()));
  insertNewAction( action );

  action = new KAction(KIcon("x-office-address-book"), i18n("New KJots Book"), this);
  actionCollection()->addAction("new_kjots_book", action );
//   connect(action, SIGNAL(triggered(bool)), SLOT( newBook()));
  insertNewAction( action );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KJotsUniqueAppHandler>(), this );
}

KJotsPlugin::~KJotsPlugin()
{
}

bool KJotsPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KJotsPlugin::invisibleToolbarActions() const
{
  return QStringList() << "new_page" << "new_book" ;
}

KParts::ReadOnlyPart* KJotsPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return 0;

  connect( part, SIGNAL(showPart()), this, SLOT(showPart()) );

  return part;
}

void KJotsPlugin::newPage()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return;

  KJotsPart* kjotsPart = dynamic_cast<KJotsPart*>(part);
  if(kjotsPart) {
//     kjotsPart->newPage();
  }
}

void KJotsPlugin::showPart()
{
  core()->selectPlugin(this);
}

void KJotsUniqueAppHandler::loadCommandLineOptions()
{
//  No command line args to load.
}

