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
#include <ktempfile.h>

#include <kabc/addressee.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/icaldrag.h>
#include <libkcal/calendarlocal.h>

#include <libkdepim/kvcarddrag.h>

#include <kmail/kmail_part.h>
#include <kmail/kmkernel.h>

#include "core.h"
#include "summarywidget.h"

#include "kmail_plugin.h"

using namespace KCal;

typedef KGenericFactory<KMailPlugin, Kontact::Core> KMailPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kmailplugin,
                            KMailPluginFactory( "kontact_kmailplugin" ) )

KMailPlugin::KMailPlugin(Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "kmail" ),
    mStub( 0 )
{
  setInstance( KMailPluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Message..." ), "mail_new",
                   0, this, SLOT( slotNewMail() ), actionCollection(),
                   "new_mail" ) );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KMailUniqueAppHandler>(), this );
}

bool KMailPlugin::canDecodeDrag( QMimeSource *qms )
{
  return ( ICalDrag::canDecode( qms ) ||
           VCalDrag::canDecode( qms ) ||
           KVCardDrag::canDecode( qms ) );
}

void KMailPlugin::processDropEvent( QDropEvent * de )
{
  kdDebug() << k_funcinfo << endl;
  CalendarLocal cal;
  KABC::Addressee::List list;

  if ( VCalDrag::decode( de, &cal ) || ICalDrag::decode( de, &cal ) ) {
    KTempFile tmp( locateLocal( "tmp", "incidences-" ), ".ics" );
    cal.save( tmp.name() );
    openComposer( KURL::fromPathOrURL( tmp.name() ) );
  }
  else if ( KVCardDrag::decode( de, list ) ) {
    KABC::Addressee::List::Iterator it;
    QStringList to;
    for ( it = list.begin(); it != list.end(); ++it ) {
      to.append( ( *it ).fullEmail() );
    }
    openComposer( to.join(", ") );
  }

}

void KMailPlugin::openComposer( const KURL& attach )
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( mStub );
  if ( mStub ) {
    if ( attach.isValid() )
      mStub->openComposer( "", "", "", "", "", false, KURL(), attach );
    else
      mStub->newMessage();
  }
}

void KMailPlugin::openComposer( const QString& to )
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( mStub );
  if ( mStub ) {
      mStub->openComposer( to, "", "", "", "", 0 ); 
  }
}

void KMailPlugin::slotNewMail()
{
  openComposer( QString::null );
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

KParts::ReadOnlyPart* KMailPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return 0;

  mStub = new KMailIface_stub( dcopClient(), "kmail", "KMailIface" );

  return part;
}

QStringList KMailPlugin::invisibleToolbarActions() const
{
  return QStringList( "new_message" );
}

bool KMailPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

Kontact::Summary *KMailPlugin::createSummaryWidget( QWidget *parent )
{
  return new SummaryWidget( this, parent );
}

////

#include "../../../kmail/kmail_options.h"
void KMailUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( kmail_options );
}

int KMailUniqueAppHandler::newInstance()
{
    // Ensure part is loaded
    (void)plugin()->part();
    DCOPRef kmail( "kmail", "KMailIface" );
    DCOPReply reply = kmail.call( "handleCommandLine", false );
    if ( reply.isValid() ) {
        bool handled = reply;
        //kdDebug(5602) << k_funcinfo << "handled=" << handled << endl;
        if ( !handled ) // no args -> simply bring kmail plugin to front
            return Kontact::UniqueAppHandler::newInstance();
    }
    return 0;
}

bool KMailPlugin::queryClose() const {
  KMailIface_stub stub( kapp->dcopClient(), "kmail", "KMailIface" );
  bool canClose=stub.canQueryClose();
  return canClose;
}

#include "kmail_plugin.moc"
