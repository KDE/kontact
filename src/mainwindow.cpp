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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include <qhbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qsplitter.h>
#include <qobjectlist.h>

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
#include <kguiitem.h>
#include <kpopupmenu.h>
#include <kshortcut.h>
#include <kcmultidialog.h>

#include <dcopclient.h>

#include <infoextension.h>

#include "kpplugin.h"

#include "mainwindow.h"
#include "sidepane.h"

using namespace Kontact;

MainWindow::MainWindow()
  : Kontact::Core()
{
  // create the GUI
  QHBox *box = new QHBox( this );
  box->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  m_curPlugin = 0L;
  
  m_splitter = new QSplitter( box );

  m_sidePane = new SidePane( m_splitter );
  m_sidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                          QSizePolicy::Preferred ) );
  connect( m_sidePane, SIGNAL( showPart( KParts::Part*, Kontact::Plugin* ) ),
           SLOT( showPart( KParts::Part*, Kontact::Plugin* ) ) );

  QVBox *vBox = new QVBox( m_splitter );
  vBox->setSpacing( 0 );

  initHeaderWidget( vBox );

  m_stack = new QWidgetStack( vBox );

  setCentralWidget( box );

  statusBar()->show();

  // prepare the part manager
  m_partManager = new KParts::PartManager( this );
  connect( m_partManager, SIGNAL( activePartChanged( KParts::Part* ) ),
           this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

  setupActions();

  loadPlugins();

  setXMLFile( "kontactui.rc" );

  createGUI( 0 );

  m_sidePane->invokeFirstEntry();

  resize( 600, 400 ); // initial size
  setAutoSaveSettings();

  loadSettings();
}

MainWindow::~MainWindow()
{
  saveSettings();
  
  QPtrList<KParts::Part> parts = *m_partManager->parts();
  parts.setAutoDelete( true );
  parts.clear();
}

void MainWindow::setupActions()
{
  (void) KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
  (void) KStdAction::preferences( this, SLOT( slotPreferences() ), actionCollection() );
  m_newActions = new KToolBarPopupAction( KGuiItem(i18n( "New" ), "filenew2"), 
		  KShortcut(), this, SLOT(slotNewClicked()),actionCollection(), "action_new" );
}

void MainWindow::initHeaderWidget(QVBox *vBox)
{
  // Initiate the headerWidget
  m_headerFrame = new QHBox( vBox );
  m_headerFrame->setSizePolicy( QSizePolicy::MinimumExpanding,
                                QSizePolicy::Maximum );
  m_headerFrame->setSpacing( 0 );

  m_headerText = new QLabel( m_headerFrame );
  m_headerText->setSizePolicy( QSizePolicy::MinimumExpanding,
                               QSizePolicy::Preferred );
  m_headerText->setFrameShape( QFrame::ToolBarPanel );

  m_headerPixmap = new QLabel( m_headerFrame );
  m_headerPixmap->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
  m_headerPixmap->setFrameShape( QFrame::ToolBarPanel );
  m_headerPixmap->setAlignment( AlignRight );

  connect( this, SIGNAL( textChanged( const QString& ) ),
           m_headerText, SLOT( setText( const QString& ) ) );
  connect( this, SIGNAL( iconChanged( const QPixmap& ) ),
           m_headerPixmap, SLOT( setPixmap( const QPixmap& ) ) );

  QFont fnt( m_sidePane->font() );
  fnt.setBold( true );
  fnt.setPointSize( m_sidePane->font().pointSize() + 3 );
  m_headerText->setFont( fnt );

  m_lastInfoExtension = 0L;
}

void MainWindow::loadPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1( "Kontact/Plugin" ), QString::null );

  for ( KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it )
  {
    kdDebug(5600) << "Loading Plugin: " << (*it)->name() << endl;
    Kontact::Plugin *plugin = KParts::ComponentFactory
      ::createInstanceFromService<Kontact::Plugin>( *it, this );
    if ( !plugin )
      continue;
	KAction *action;
	QPtrList<KAction> *actionList = plugin->newActions();
		
	for(action = actionList->first(); action; action = actionList->next()){
      kdDebug() << "Plugging " << action->name() << endl;
      action->plug(m_newActions->popupMenu());
	}
    addPlugin( plugin );
  }
}

void MainWindow::addPlugin( Kontact::Plugin *plugin )
{
  kdDebug(5600) << "Added plugin" << endl;

  // merge the plugins GUI into the main window
  insertChildClient( plugin );
  m_plugins.append( plugin );
  m_sidePane->addEntry( plugin );
}

void MainWindow::addPart( KParts::Part *part )
{
  m_partManager->addPart( part, false );

  if ( part->widget() )
    m_stack->addWidget( part->widget(), 0 );
}

void MainWindow::slotActivePartChanged( KParts::Part *part )
{
  if ( !part )  // part can be 0 at shutdown
    return;

  kdDebug(5600) << "Part activated: " << part << " with stack id. "
      << m_stack->id( part->widget() )<< endl;
  QObjectList *l = part->queryList( "KParts::InfoExtension" );
  KParts::InfoExtension *ie = static_cast<KParts::InfoExtension*>( l->first() );
  if ( ie != 0L )
  {
    disconnect( m_lastInfoExtension, SIGNAL( textChanged( const QString& ) ),
                m_headerText, SLOT( setText( const QString& ) ) );
    connect( ie, SIGNAL( textChanged( const QString& ) ),
             m_headerText, SLOT( setText( const QString& ) ) );
    disconnect( m_lastInfoExtension, SIGNAL( iconChanged( const QPixmap& ) ),
                m_headerPixmap, SLOT( setPixmap( const QPixmap& ) ) );
    connect( ie, SIGNAL( iconChanged( const QPixmap& ) ),
             m_headerPixmap, SLOT( setPixmap( const QPixmap& ) ) );
  }
  else
  {
    m_headerText->setText( QString::null );
    m_headerPixmap->setPixmap( QPixmap() );
  }
  m_lastInfoExtension = ie;

  createGUI(part);
}

void MainWindow::slotNewClicked()
{
  KAction *action = m_curPlugin->newActions()->first();
  if (action) action->activate();
}

void MainWindow::showPart( KParts::Part* part, Kontact::Plugin *plugin )
{
  QPtrList<KParts::Part> *partList = const_cast<QPtrList<KParts::Part>*>( m_partManager->parts() );
  if ( partList->find( part ) == -1 )
    addPart( part );

  m_partManager->setActivePart( part );
  QWidget* view = part->widget();
  Q_ASSERT( view );

  if ( view )
  {
    m_stack->raiseWidget( view );
    view->show();
    view->setFocus();
    m_curPlugin = plugin;
    KAction *action = plugin->newActions()->first();
    if (action) {
      // ##FIXME: Doesn't work for some reason..
      m_newActions->setIconSet(action->iconSet());
      m_newActions->setText(action->text());
	}
  }
}

void MainWindow::loadSettings()
{
  KConfig *config = kapp->config();
  KConfigGroupSaver saver( config, "General" );
  m_splitter->setSizes( config->readIntListEntry( "SideBarSize" ) );
}

void MainWindow::saveSettings()
{
  KConfig *config = kapp->config();
  KConfigGroupSaver saver( config, "General" );
  config->writeEntry( "SideBarSize", m_splitter->sizes() );
}

void MainWindow::slotQuit()
{
  close();
}

void MainWindow::slotPreferences()
{
  KCMultiDialog* dialog = new KCMultiDialog( "PIM", this, "KontactPreferences" );

  QStringList modules;

  // find all all modules for all plugins
  QPtrListIterator<Kontact::Plugin> pit( m_plugins );
  for( ; pit.current(); ++pit )
  {
    QStringList tmp = pit.current()->configModules();
    if( !tmp.isEmpty() )
      modules += tmp;
  }

  // add them all
  QStringList::iterator mit;
  for ( mit = modules.begin(); mit != modules.end(); ++mit )
    dialog->addModule( *mit );

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
      kdDebug(5600) << "found interface for " << serviceType << endl;
      if ( dcopService )
        *dcopService = it.current()->dcopClient()->appId();
      kdDebug(5600) << "appId=" << it.current()->dcopClient()->appId() << endl;
      return 0; // success
    }
  }
  kdDebug(5600) << "Didn't find dcop interface, falling back to external process" << endl;
  return KDCOPServiceStarter::startServiceFor( serviceType, constraint, preferences, error, dcopService, flags );
}

#include "mainwindow.moc"
