/*
    This file is part of Kontact.
    Copyright (c) 2003 Kontact Developer

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

#include <qwidget.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "core.h"
#include "summarywidget.h"

#include "kmail_plugin.h"

typedef KGenericFactory<KMailPlugin, Kontact::Core> KMailPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kmailplugin,
                            KMailPluginFactory( "kontact_kmailplugin" ) )

KMailPlugin::KMailPlugin(Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "kmail" ),
    mStub( 0 )
{
  setInstance( KMailPluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Mail" ), BarIcon( "mail_new2" ),
			             0, this, SLOT( slotNewMail() ), actionCollection(),
                   "new_mail" ) );
}


void KMailPlugin::slotNewMail()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( mStub );
  if ( mStub )
    mStub->openComposer( "", "", "", "", "", false, 0 );
}

KMailPlugin::~KMailPlugin()
{
}

bool KMailPlugin::createDCOPInterface( const QString& serviceType )
{
  if ( serviceType == "DCOP/ResourceBackend/IMAP" ) {
    if ( part() )
      return true;
  }

  return false;
}

QString KMailPlugin::tipFile() const
{
  QString file = ::locate("data", "kmail/tips");
  return file;
}

KParts::Part* KMailPlugin::createPart()
{
  KParts::Part *part = loadPart();
  if ( !part ) return 0;

  KAction *action = part->actionCollection()->action( "new_message" );
  if ( action )
    part->actionCollection()->take( action );

  mStub = new KMailIface_stub( dcopClient(), "kmail", "KMailIface" );
  return part;
}

bool KMailPlugin::isRunningStandalone()
{
  DCOPClient* dc = kapp->dcopClient();

  return (dc->isApplicationRegistered("kmail")) &&
         (!dc->remoteObjects("kontact").contains("KMailIface"));
}

Kontact::Summary *KMailPlugin::createSummaryWidget( QWidget *parent )
{
  return new SummaryWidget( this, parent );
}

#include "kmail_plugin.moc"
