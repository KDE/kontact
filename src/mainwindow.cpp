/*
    This file is part of Kontact
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
#include <qwhatsthis.h>
#include <qsplitter.h>

#include <kapplication.h>
#include <kconfig.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <klistbox.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kshortcut.h>
#include <kparts/componentfactory.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kcmultidialog.h>

#include <dcopclient.h>

#include "kpplugin.h"

#include "mainwindow.h"
#include "sidepane.h"

using namespace Kontact;

MainWindow::MainWindow()
  : Kontact::Core()
{
  // create the GUI
  QHBox *box = new QHBox(this);
  box->setFrameStyle(  QFrame::Panel | QFrame::Sunken );

  QSplitter *splitter = new QSplitter(box);
  
  m_sidePane = new SidePane(splitter);
  m_sidePane->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
  connect(m_sidePane, SIGNAL(showPart(KParts::Part*)), SLOT(showPart(KParts::Part*)));
  
  m_stack = new QWidgetStack(splitter);
 
  setCentralWidget(box);

  statusBar()->show();

  // prepare the part manager
  m_partManager = new KParts::PartManager(this);
  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(activePartChanged(KParts::Part *)));

  setupActions();

  loadSettings();

  loadPlugins();

  setXMLFile("kontactui.rc");

  createGUI(0);
  
  m_sidePane->invokeFirstEntry();
  
  resize(600, 400); // initial size
  setAutoSaveSettings();
}


MainWindow::~MainWindow()
{
    QPtrList<KParts::Part> parts = *m_partManager->parts();
    parts.setAutoDelete(true);
    parts.clear();
}

void MainWindow::loadSettings()
{
}

void MainWindow::saveSettings()
{
}

void MainWindow::setupActions()
{
  (void) KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");
  (void) KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection(), "settings_preferences");
  m_newActions = new KActionMenu( i18n("New"), BarIcon("mail_generic"), actionCollection(), "action_new" );
}


void MainWindow::insertNewAction(KAction *action)
{
  m_newActions->insert(action);
}

void MainWindow::loadPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("Kontact/Plugin"), QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
  {
    kdDebug() << "Loading Plugin: " << (*it)->name() << endl;
    Kontact::Plugin *plugin = KParts::ComponentFactory
      ::createInstanceFromService<Kontact::Plugin>(*it, this);
    if (!plugin)
      continue;

    addPlugin(plugin);
  }
}


void MainWindow::addPlugin(Kontact::Plugin *plugin)
{
  kdDebug() << "Added plugin" << endl;

  // merge the plugins GUI into the main window
  insertChildClient(plugin);
  m_plugins.append(plugin);
  m_sidePane->addEntry(plugin);
}

void MainWindow::addPart(KParts::Part *part)
{
  m_partManager->addPart(part, false);

  if (part->widget())
    m_stack->addWidget(part->widget(), 0);
}

void MainWindow::activePartChanged(KParts::Part *part)
{
  kdDebug() << "Part activated: " << part << endl;
  createGUI(part);
}

void MainWindow::showPart(KParts::Part* part)
{
    
  QPtrList<KParts::Part> parts = *m_partManager->parts();
//  if (!parts.find(part))
      addPart(part);

  m_partManager->setActivePart( part );
  QWidget* view = part->widget();
  Q_ASSERT(view);
  if ( view )
  {
    m_stack->raiseWidget( view );
    view->show();
    view->setFocus();
  }
}


void MainWindow::slotQuit()
{
  close();
}

void MainWindow::slotPreferences()
{
  KCMultiDialog* dialog = new KCMultiDialog("PIM", this, "KontactPreferences");

  QStringList modules;


  // find all all modules for all plugins 
  QPtrListIterator<Kontact::Plugin> pit(m_plugins);
  for(; pit.current(); ++pit)
  {
     QStringList tmp = pit.current()->configModules();
     if(!tmp.isEmpty())
         modules += tmp;
  }
  

  // add them all  
  QStringList::iterator mit;
  for (mit = modules.begin(); mit != modules.end(); ++mit)
    dialog->addModule((*mit));

  dialog->show();
  dialog->raise();
}

int MainWindow::startServiceFor( const QString& serviceType,
                           const QString& constraint,
                           const QString& preferences,
                           QString *error, QCString* dcopService, int flags )
{
  QPtrListIterator<Kontact::Plugin> it( m_plugins );
  for ( ; it.current() ; ++it )
  {
    if ( it.current()->createDCOPInterface( serviceType ) ) {
      kdDebug() << "found interface for " << serviceType << endl;
      if ( dcopService )
        *dcopService = it.current()->dcopClient()->appId();
      kdDebug() << "appId=" << it.current()->dcopClient()->appId() << endl;
      return 0; // success
    }
  }
  kdDebug() << "Didn't find dcop interface, falling back to external process" << endl;
  return KDCOPServiceStarter::startServiceFor( serviceType, constraint, preferences, error, dcopService, flags );
}

#include "mainwindow.moc"

// vim: ts=4 sw=4 et
