/*
    This file is part of KDE Kontact.

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
#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>

#include "core.h"

#include "test_plugin.h"
#include "test_part.h"

typedef KGenericFactory< TestPlugin, Kontact::Core > TestPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkptestplugin, TestPluginFactory( "kptestplugin" ) );

TestPlugin::TestPlugin(Kontact::Core *_core, const char *name, const QStringList &)
  : Kontact::Plugin(i18n("Test"), "test", _core, _core, name)
{
  setInstance(TestPluginFactory::instance());

  insertNewAction(new KAction("Test", 0, this, SLOT(slotTestMenu()), actionCollection(), "edit_test"));

  setXMLFile("kptestplugin.rc");
}

TestPlugin::~TestPlugin()
{
}

void TestPlugin::slotTestMenu()
{
  core()->statusBar()->message("Test menu activated");
}

KParts::Part* TestPlugin::createPart()
{
  return new TestPart(this, "testpart");
}

#include "test_plugin.moc"
