#include <qhbox.h>
#include <qcombobox.h>

#include <kapp.h>
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

#include "kpplugin.h"


#include "navigator.h"
#include "core.h"


Core::Core()
  : Kaplan::Core()
{
  // create the GUI
  QHBox *box = new QHBox(this);	

  m_navigator = new Navigator(box);
  m_navigator->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
  
  m_stack = new QWidgetStack(box);

  setCentralWidget(box);
 
  // prepare the part manager
  m_partManager = new KParts::PartManager(this);
  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(activePartChanged(KParts::Part *))); 

  setupActions();

  loadSettings();
  
  loadPlugins();

  setXMLFile("kaplanrc");

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
  m_newActions->insert(new KAction( i18n( "Note" ), BarIcon( "knotes" ), 0, this, SLOT(""), actionCollection(), "newNote" ) );
  m_newActions->insert(new KAction( i18n( "Mail" ), BarIcon( "mail_generic" ), 0, this, SLOT(""), actionCollection(), "newMail" ) );
  m_newActions->insert(new KAction( i18n( "Contact" ), BarIcon( "contact" ), 0, this, SLOT(""), actionCollection(), "newContact" ) );
  m_newActions->insert(new KAction( i18n( "Todo" ), BarIcon( "todo" ), 0, this, SLOT(""), actionCollection(), "newTodo" ) );
  m_newActions->setDelayed( true );
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
