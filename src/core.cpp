/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <qhbox.h>
#include <qcombobox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <klistbox.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kshortcut.h>
#include <kparts/componentfactory.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstatusbar.h>

#include "kpplugin.h"


#include "navigator.h"
#include "core.h"


Core::Core()
  : Kaplan::Core()
{
  // create the GUI
  QHBox *box = new QHBox(this);	
  box->setFrameStyle(  QFrame::Panel | QFrame::Sunken );
  
  m_navigator = new Navigator(box);
  m_navigator->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
  
  m_stack = new QWidgetStack(box);

  setCentralWidget(box);

  statusBar()->show();
  // prepare the part manager
  m_partManager = new KParts::PartManager(this);
  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(activePartChanged(KParts::Part *))); 

  setupActions();

  loadSettings();
  
  loadPlugins();

  setXMLFile("kaplanui.rc");

  createGUI(0);
}


Core::~Core()
{
	saveSettings();	
}

void Core::loadSettings()
{
	KConfig* config = kapp->config();
	config->setGroup("General");
	resize( config->readSizeEntry("Dimensions", new QSize(600, 400)) );
}
	
void Core::saveSettings()
{
	KConfig* config = kapp->config();
	config->setGroup("General");
	config->writeEntry("Dimensions", size());
}

void Core::setupActions()
{
  (void) KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");
  m_newActions = new KActionMenu( i18n("New"), BarIcon("mail_generic"), actionCollection(), "action_new" );
}


void Core::insertNewAction(KAction *action)
{
  m_newActions->insert(action);
}

void Core::loadPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("Kaplan/Plugin"), QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
  {
    kdDebug() << "Loading Plugin: " << (*it)->name() << endl;
    Kaplan::Plugin *plugin = KParts::ComponentFactory
      ::createInstanceFromService<Kaplan::Plugin>(*it, this);
    if (!plugin)
      continue;

    addPlugin(plugin);
  }
}


void Core::addPlugin(Kaplan::Plugin *plugin)
{
  kdDebug() << "Added plugin" << endl;

  // merge the plugins GUI into the main window
  insertChildClient(plugin);
}


void Core::addPart(KParts::Part *part)
{
  m_partManager->addPart(part);

  if (part->widget())
    m_stack->addWidget(part->widget(), 0);
}


void Core::activePartChanged(KParts::Part *part)
{
  kdDebug() << "Part activated: " << part << endl;
  createGUI(part);
}


void Core::showView(QWidget *view)
{
  m_stack->raiseWidget(view);
  view->show();
  view->setFocus();
}


void Core::slotQuit()
{
  kapp->quit();
}


void Core::addMainEntry(QString text, QString icon, QObject *receiver, const char *member)
{
  m_navigator->addEntry(text, icon, receiver, member);
}


#include "core.moc"

// vim: ts=4 sw=4 et
