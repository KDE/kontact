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
#include <qwhatsthis.h>

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

#include "kpplugin.h"


#include "navigator.h"
#include "core.h"
#include <dcopclient.h>


Core::Core()
  : Kaplan::Core()
{
  // create the GUI
  QHBox *box = new QHBox(this);
  box->setFrameStyle(  QFrame::Panel | QFrame::Sunken );

  m_navigator = new Navigator(box);
  m_navigator->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

  QWhatsThis::add(m_navigator, i18n("Use this bar to invoke a specific groupware part"));

  m_stack = new QWidgetStack(box);

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

  m_navigator->setCurrentItem(0);

  resize(600, 400); // initial size
  setAutoSaveSettings();
}


Core::~Core()
{
    QPtrList<KParts::Part> parts = *m_partManager->parts();
    parts.setAutoDelete(true);
    parts.clear();
}

void Core::loadSettings()
{
}

void Core::saveSettings()
{
}

void Core::setupActions()
{
  (void) KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");
  (void) KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection(), "settings_preferences");
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
  m_plugins.append(plugin);
}


void Core::addPart(KParts::Part *part)
{
  m_partManager->addPart(part, false);

  if (part->widget())
    m_stack->addWidget(part->widget(), 0);
}


void Core::activePartChanged(KParts::Part *part)
{
  kdDebug() << "Part activated: " << part << endl;
  createGUI(part);
}


void Core::showPart(KParts::Part* part)
{
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


void Core::slotQuit()
{
  close();
}

void Core::slotPreferences()
{
  KCMultiDialog* dialog = new KCMultiDialog("PIM", this, "KaplanPreferences");

  QStringList modules;


  // find all all modules for all plugins 
  QPtrListIterator<Kaplan::Plugin> pit(m_plugins);
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

void Core::addMainEntry(QString text, QString icon, QObject *receiver, const char *member)
{
  m_navigator->addEntry(text, icon, receiver, member);
}

int Core::startServiceFor( const QString& serviceType,
                           const QString& constraint,
                           const QString& preferences,
                           QString *error, QCString* dcopService, int flags )
{
  QPtrListIterator<Kaplan::Plugin> it( m_plugins );
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

#include "core.moc"

// vim: ts=4 sw=4 et
