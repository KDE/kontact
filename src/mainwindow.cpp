/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#include <qimage.h>

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
#include <khelpmenu.h>

#include <dcopclient.h>

#include <infoextension.h>

#include "plugin.h"

#include "prefs.h"
#include "mainwindow.h"
#include "sidepane.h"
#include "iconsidepane.h"
#include "aboutdialog.h"

using namespace Kontact;

MainWindow::MainWindow()
  : Kontact::Core(), m_topWidget( 0 ), m_headerText( 0 ), m_headerPixmap( 0 ), m_splitter( 0 ), 
    m_currentPlugin( 0 ), m_lastInfoExtension( 0 ), m_aboutDialog( 0 )
{
  m_plugins.setAutoDelete( true );

  statusBar()->show();

  initWidgets();
 
  // prepare the part manager
  m_partManager = new KParts::PartManager( this );
  connect( m_partManager, SIGNAL( activePartChanged( KParts::Part* ) ),
           this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

  setupActions();

  setHelpMenuEnabled( false );

  KHelpMenu *helpMenu = new KHelpMenu( this, 0, true, actionCollection() );
  connect( helpMenu, SIGNAL( showAboutApplication() ),
           SLOT( showAboutDialog() ) );

  loadPlugins();

  setXMLFile( "kontactui.rc" );

  createGUI( 0 );

  resize( 600, 400 ); // initial size
  setAutoSaveSettings();

  if ( m_sidePane )
    m_sidePane->updatePlugins();

  loadSettings();
}

MainWindow::~MainWindow()
{
  saveSettings();

  QPtrList<KParts::Part> parts = *m_partManager->parts();
  parts.setAutoDelete( true );
  parts.clear();

  Prefs::self()->writeConfig();
}

void MainWindow::initWidgets()
{
  QHBox *topWidget = new QHBox( this );
  topWidget->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  m_topWidget = topWidget;

  setCentralWidget( m_topWidget );

  mSidePaneType = Prefs::self()->mSidePaneType;

  m_splitter = new QSplitter( m_topWidget );

  switch ( mSidePaneType ) {
    case Prefs::SidePaneIcons:
      m_sidePane = new IconSidePane( this, m_splitter );
      break;
    default:
      kdError() << "Invalid SidePaneType: " << mSidePaneType << endl;
    case Prefs::SidePaneBars:
      m_sidePane = new SidePane( this, m_splitter );
      m_sidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                                              QSizePolicy::Preferred ) );
      break;
  }
  connect( m_sidePane, SIGNAL( pluginSelected( Kontact::Plugin* ) ),
           SLOT( selectPlugin( Kontact::Plugin* ) ) );

  QVBox *vBox = new QVBox( m_splitter );

  initHeaderWidget( vBox );
  if ( mSidePaneType != Prefs::SidePaneBars )
    m_headerFrame->hide();

  vBox->setSpacing( 0 );

  m_stack = new QWidgetStack( vBox );
}

void MainWindow::setupActions()
{
  (void) KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
  m_newActions = new KToolBarPopupAction( KGuiItem(i18n( "New" ), "filenew2"), 
		  KShortcut(), this, SLOT(slotNewClicked()),actionCollection(), "action_new" );

  new KAction( i18n("Configure Kontact..."), 0, this, SLOT( slotPreferences() ),
               actionCollection(), "settings_configure_kontact" );
}

void MainWindow::initHeaderWidget(QVBox *vBox)
{
  // Initiate the headerWidget
  m_headerFrame = new QHBox( vBox );
  m_headerFrame->setSizePolicy( QSizePolicy::MinimumExpanding,
                                QSizePolicy::Maximum );
  m_headerFrame->setSpacing( 0 );
  m_headerFrame->setFixedHeight( 22 );

  m_headerText = new QLabel( m_headerFrame );
  m_headerText->setSizePolicy( QSizePolicy::MinimumExpanding,
                               QSizePolicy::Preferred );
  m_headerText->setPaletteForegroundColor( colorGroup().light() );
  m_headerText->setPaletteBackgroundColor( colorGroup().dark() );
  
  m_headerPixmap = new QLabel( m_headerFrame );
  m_headerPixmap->setSizePolicy( QSizePolicy::Maximum,
                                 QSizePolicy::Preferred );
  m_headerPixmap->setAlignment( AlignRight|AlignVCenter );
  m_headerPixmap->setPaletteBackgroundColor( colorGroup().dark() );

  connect( this, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( setHeaderText( const QString& ) ) );
  connect( this, SIGNAL( iconChanged( const QPixmap& ) ),
           this, SLOT( setHeaderPixmap( const QPixmap& ) ) );

  QFont fnt( m_sidePane->font() );
  fnt.setBold( true );
  fnt.setPointSize( m_sidePane->font().pointSize() + 3 );
  m_headerText->setFont( fnt );
}

void MainWindow::loadPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1( "Kontact/Plugin" ), QString::null );

  QPtrList<Plugin> plugins;

  uint i;

  for ( KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it )
  {
    kdDebug(5600) << "Loading Plugin: " << (*it)->name() << endl;
    Kontact::Plugin *plugin = KParts::ComponentFactory
      ::createInstanceFromService<Kontact::Plugin>( *it, this );

    if ( !plugin )
      continue;
    
    QVariant variant = (*it)->property( "X-KDE-KontactIdentifier" );
    plugin->setIdentifier( variant.toString() );
    plugin->setTitle( (*it)->name() );
    plugin->setIcon( (*it)->icon() );

    for( i = 0; i < plugins.count(); ++i ) {
      Plugin *p = plugins.at( i );
      if ( plugin->weight() < p->weight() ) break;
    }

    plugins.insert( i, plugin );
  }

  for( i = 0; i < plugins.count(); ++ i ) {
    Plugin *plugin = plugins.at( i );

    KAction *action;
    QPtrList<KAction> *actionList = plugin->newActions();
	
    for(action = actionList->first(); action; action = actionList->next()){
      kdDebug() << "Plugging " << action->name() << endl;
      action->plug(m_newActions->popupMenu());
    }
    addPlugin( plugin );
  }
}

void MainWindow::unloadPlugins()
{
  QPtrList<KParts::Part> parts = *m_partManager->parts();
  parts.setAutoDelete( true );
  parts.clear();

  m_plugins.clear();
}

void MainWindow::addPlugin( Kontact::Plugin *plugin )
{
  kdDebug(5600) << "Added plugin" << endl;

  m_plugins.append( plugin );

  // merge the plugins GUI into the main window
  insertChildClient( plugin );
}

void MainWindow::addPart( KParts::Part *part )
{
  if ( part->widget() )
    m_stack->addWidget( part->widget(), 0 );

  m_partManager->addPart( part, false );
}

void MainWindow::slotActivePartChanged( KParts::Part *part )
{
  if ( !part )
    return;

  if ( m_lastInfoExtension ) {
    disconnect( m_lastInfoExtension, SIGNAL( textChanged( const QString& ) ),
                this, SLOT( setHeaderText( const QString& ) ) );
    disconnect( m_lastInfoExtension, SIGNAL( iconChanged( const QPixmap& ) ),
                this, SLOT( setHeaderPixmap( const QPixmap& ) ) );
  }

  kdDebug(5600) << "Part activated: " << part << " with stack id. "
      << m_stack->id( part->widget() )<< endl;
  QObjectList *l = part->queryList( "KParts::InfoExtension" );
  KParts::InfoExtension *ie = 0;
  if ( l )
      ie = static_cast<KParts::InfoExtension*>( l->first() );
  delete l;
  if ( ie ) {
    connect( ie, SIGNAL( textChanged( const QString& ) ),
             SLOT( setHeaderText( const QString& ) ) );
    connect( ie, SIGNAL( iconChanged( const QPixmap& ) ),
             SLOT( setHeaderPixmap( const QPixmap& ) ) );
  }

  m_lastInfoExtension = ie;

  InfoExtData data = m_infoExtCache[ ie ];
  setHeaderPixmap( data.pixmap );
  setHeaderText( data.text );

  createGUI( part );
}

void MainWindow::slotNewClicked()
{
  KAction *action = m_currentPlugin->newActions()->first();
  if ( action ) action->activate();
}

void MainWindow::selectPlugin( Kontact::Plugin *plugin )
{
  if ( !plugin )
    return;

  if ( m_sidePane )
    m_sidePane->selectPlugin( plugin );

  KParts::Part *part = plugin->part();

  QPtrList<KParts::Part> *partList = const_cast<QPtrList<KParts::Part>*>( m_partManager->parts() );
  if ( partList->find( part ) == -1 )
    addPart( part );

  m_partManager->setActivePart( part );
  QWidget *view = part->widget();
  Q_ASSERT( view );

  if ( view )
  {
    m_stack->raiseWidget( view );
    view->show();
    view->setFocus();
    m_currentPlugin = plugin;
    KAction *action = plugin->newActions()->first();
    if ( action ) {
      // ##FIXME: Doesn't work for some reason..
      m_newActions->setIconSet( action->iconSet() );
      m_newActions->setText( action->text() );
    }
  }
}

void MainWindow::selectPlugin( const QString &pluginName )
{
  for ( Kontact::Plugin *plugin = m_plugins.first(); plugin; plugin = m_plugins.next() ) {
    if ( plugin->identifier() == pluginName ) {
      selectPlugin( plugin );
      return;
    }
  }
}

void MainWindow::loadSettings()
{
  if ( m_splitter )
    m_splitter->setSizes( Prefs::self()->mSidePaneSplitter );

  selectPlugin( Prefs::self()->mActivePlugin );
}

void MainWindow::saveSettings()
{
  if ( m_splitter )
    Prefs::self()->mSidePaneSplitter = m_splitter->sizes();

  if ( m_currentPlugin )
    Prefs::self()->mActivePlugin = m_currentPlugin->identifier();
}

void MainWindow::slotQuit()
{
  close();
}

void MainWindow::slotPreferences()
{
  KCMultiDialog *dialog = new KCMultiDialog( "PIM", this, "KontactPreferences" );
  connect( dialog, SIGNAL( applyClicked() ), SLOT( updateConfig() ) );
  connect( dialog, SIGNAL( okClicked() ), SLOT( updateConfig() ) );

  QStringList modules;

  modules.append( "PIM/kontactconfig.desktop" );

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

void MainWindow::setHeaderText( const QString &text )
{
  m_infoExtCache[ m_lastInfoExtension ].text = text;
  m_headerText->setText( text );
}

void MainWindow::setHeaderPixmap( const QPixmap &pixmap )
{
  QPixmap pm( pixmap );
  
  if ( pm.height() > 22 || pm.width() > 22 ) {
    QImage img;
    img = pixmap;
    pm = img.smoothScale( 22, 22, QImage::ScaleMin );
  }

  m_infoExtCache[ m_lastInfoExtension ].pixmap = pm;  
  m_headerPixmap->setPixmap( pm );
}

void MainWindow::updateConfig()
{
  kdDebug() << "MainWindow::updateConfig()" << endl;

  if ( Prefs::self()->mSidePaneType != mSidePaneType ) {
    kdDebug() << "Recreate top widget." << endl;

    mSidePaneType = Prefs::self()->mSidePaneType;

    saveSettings();
    slotActivePartChanged( 0 );

    delete m_sidePane;

    switch ( mSidePaneType ) {
      case Prefs::SidePaneIcons:
        m_sidePane = new IconSidePane( this, m_splitter );
        m_headerFrame->hide();
        break;
      default:
        kdError() << "Invalid SidePaneType: " << mSidePaneType << endl;
      case Prefs::SidePaneBars:
        m_sidePane = new SidePane( this, m_splitter );
        m_headerFrame->show();
        break;
    }

    m_sidePane->setSizePolicy( QSizePolicy( QSizePolicy::Maximum,
                               QSizePolicy::Preferred ) );

    connect( m_sidePane, SIGNAL( pluginSelected( Kontact::Plugin* ) ),
             SLOT( selectPlugin( Kontact::Plugin* ) ) );

    m_splitter->moveToFirst( m_sidePane );

    m_sidePane->show();
    m_sidePane->updatePlugins();

    loadSettings();
  }
}

void MainWindow::showAboutDialog()
{
  if ( !m_aboutDialog ) {
    m_aboutDialog = new AboutDialog( this );
  }

  m_aboutDialog->show();
  m_aboutDialog->raise();
}

#include "mainwindow.moc"
