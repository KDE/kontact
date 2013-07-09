/*
  This file is part of Kontact.

  Copyright (c) 2003 Zack Rusin <zack@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "knode_plugin.h"
#include "knodeinterface.h"
#include "knode_options.h"

#include <KontactInterface/Core>

#include <KAction>
#include <KActionCollection>
#include <KIcon>
#include <KLocale>

EXPORT_KONTACT_PLUGIN( KNodePlugin, knode )

KNodePlugin::KNodePlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "knode" ), m_interface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action =
    new KAction( KIcon( QLatin1String("mail-message-new") ),
                 i18nc( "@action:inmenu", "New Article..." ), this );
  actionCollection()->addAction( QLatin1String("post_article"), action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ) );
  action->setHelpText(
    i18nc( "@info:status", "Create a new Usenet article" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can create "
           "a new article to post on Usenet." ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotPostArticle()) );
  insertNewAction( action );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<KNodeUniqueAppHandler>(), this );
}

KNodePlugin::~KNodePlugin()
{
  delete m_interface;
  m_interface = 0;
}

bool KNodePlugin::createDBUSInterface( const QString &serviceType )
{
  Q_UNUSED( serviceType );
  return false;
}

bool KNodePlugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KNodePlugin::invisibleToolbarActions() const
{
  return QStringList() <<QLatin1String( "article_postNew" );
}

void KNodePlugin::slotPostArticle()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_interface );
  if ( m_interface ) {
    m_interface->postArticle();
  }
}

QString KNodePlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "knode/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart *KNodePlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) {
    return 0;
  }

  m_interface = new OrgKdeKnodeInterface(
    QLatin1String("org.kde.knode"), QLatin1String("/KNode"), QDBusConnection::sessionBus() );
  return part;
}

////

void KNodeUniqueAppHandler::loadCommandLineOptions()
{
  KCmdLineArgs::addCmdLineOptions( knode_options() );
}

int KNodeUniqueAppHandler::newInstance()
{
  // Ensure part is loaded
  (void)plugin()->part();
  org::kde::knode knode( QLatin1String("org.kde.knode"), QLatin1String("/KNode"), QDBusConnection::sessionBus() );
  QDBusReply<bool> reply = knode.handleCommandLine();

  if ( reply.isValid() ) {
    bool handled = reply;
    if ( !handled ) { // no args -> simply bring knode plugin to front
      return KontactInterface::UniqueAppHandler::newInstance();
    }
  }
  return 0;
}

#include "knode_plugin.moc"
