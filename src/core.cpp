#include <qsplitter.h>


#include <kapp.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <klistbox.h>
#include <kiconloader.h>
#include <kstddirs.h>


#include "kpfactory.h"
#include "kpplugin.h"


#include "navigator.h"
#include "core.h"


Core::Core()
  : KParts::MainWindow(), Kaplan::Core()
{
  // create the GUI
  QSplitter *splitter = new QSplitter(this);
  m_navigator = new Navigator(splitter);
  m_stack = new QWidgetStack(splitter);
  setCentralWidget(splitter);
 
  // prepare the part manager
  m_partManager = new KParts::PartManager(this);
  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(activePartChanged(KParts::Part *))); 

  setupActions();

  loadPlugins();

  setXMLFile("kaplanrc");

  createGUI(0);

  splitter->setResizeMode(m_navigator, QSplitter::FollowSizeHint);

  resize( 600,400 );
}


Core::~Core()
{
}


void Core::setupActions()
{
  (void) KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");
}


void Core::loadPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("Kaplan/Plugin"), QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
  {
    kdDebug() << "Loading Plugin: " << (*it)->name() << endl;
    Kaplan::Factory *factory = static_cast<Kaplan::Factory*>(KLibLoader::self()->factory((*it)->library()));
    if (!factory)
      continue;

    QStringList args;
    Kaplan::Plugin *plugin = factory->create(this, this, args);
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
