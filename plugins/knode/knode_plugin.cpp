/*
    This file is part of Kontact.
    Copyright (c) 2003 Zack Rusin

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

#include "knode_plugin.h"

#include "core.h"

#include <kapplication.h>
#include <kparts/componentfactory.h>
#include <kgenericfactory.h>
#include <kapplication.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <dcopclient.h>

#include <qwidget.h>


typedef KGenericFactory<KNodePlugin, Kontact::Core> KNodePluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_knodeplugin,
                            KNodePluginFactory( "kontact_knodeplugin" ) )


KNodePlugin::KNodePlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "knode" ), mStub(0)
{
  setInstance( KNodePluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Article..." ), "mail_new", CTRL+SHIFT+Key_A,
                   this, SLOT( slotPostArticle() ), actionCollection(), "post_article" ) );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KNodeUniqueAppHandler>(), this );
}

KNodePlugin::~KNodePlugin()
{
}

bool KNodePlugin::createDCOPInterface( const QString& /*serviceType*/ )
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
  Q_ASSERT( mStub );
  if ( mStub )
    mStub->postArticle();
}

KParts::ReadOnlyPart* KNodePlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return 0;

  mStub = new KNodeIface_stub( dcopClient(), "knode", "KNodeIface" );
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
    DCOPRef knode( "knode", "KNodeIface" );
    DCOPReply reply = knode.call( "handleCommandLine" );
#if 0
    if ( reply.isValid() ) {
        bool handled = reply;
        kdDebug(5602) << k_funcinfo << "handled=" << handled << endl;
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
