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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "knode_plugin.h"

#include "core.h"

#include <kapplication.h>
#include <kparts/componentfactory.h>
#include <kgenericfactory.h>
#include <kapplication.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kicon.h>
#include <QWidget>
#include <knodeinterface.h>

typedef KGenericFactory<KNodePlugin, Kontact::Core> KNodePluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_knodeplugin,
                            KNodePluginFactory( "kontact_knodeplugin" ) )


KNodePlugin::KNodePlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "knode" ), m_interface(0)
{
  setInstance( KNodePluginFactory::instance() );

  KAction *action = new KAction( KIcon("mail_new"), i18n( "New Article..." ), 
                                 actionCollection(), "post_article" );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_A));
  connect(action, SIGNAL(triggered(bool)), SLOT( slotPostArticle()));
  insertNewAction( action );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KNodeUniqueAppHandler>(), this );
}

KNodePlugin::~KNodePlugin()
{
}

bool KNodePlugin::createDBUSInterface( const QString& /*serviceType*/ )
{
  return false;
}

bool KNodePlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KNodePlugin::invisibleToolbarActions() const
{
  return QStringList( "article_postNew" );
}

void KNodePlugin::slotPostArticle()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_interface );
  if ( m_interface )
    m_interface->postArticle();
}

QString KNodePlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "knode/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart* KNodePlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return 0;

  m_interface = new OrgKdeKnodeInterface( "org.kde.knode", "/KNode", QDBusConnection::sessionBus() );
  return part;
}

////

#include "../../../knode/knode_options.h"
void KNodeUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( knode_options );
}

int KNodeUniqueAppHandler::newInstance()
{
    // Ensure part is loaded
    (void)plugin()->part();
    org::kde::knode knode("org.kde.knode", "/KNode", QDBusConnection::sessionBus());
    QDBusReply<bool> reply = knode.handleCommandLine();

#if 0
    if ( reply.isValid() ) {
        bool handled = reply;
        kDebug(5602) << k_funcinfo << "handled=" << handled << endl;
        if ( !handled )
#endif
    // in all cases, bring knode plugin to front
    return Kontact::UniqueAppHandler::newInstance();
#if 0
    }
    return 0;
#endif
}

#include "knode_plugin.moc"
