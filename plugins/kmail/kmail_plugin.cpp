/*
    This file is part of Kontact.
    Copyright (c) 2003 Kontact Developer <kde-pim@kde.org>

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

#include <QWidget>
#include <QDropEvent>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#include <kabc/addressee.h>

#include <kcal/vcaldrag.h>
#include <kcal/icaldrag.h>
#include <kcal/calendarlocal.h>

#include <libkdepim/kvcarddrag.h>

#include <kmail/kmail_part.h>
#include <kmail/kmkernel.h>
#include <kicon.h>
#include "core.h"
#include "summarywidget.h"

#include "kmail_plugin.h"
#include "kmailinterface.h"

using namespace KCal;

typedef KGenericFactory<KMailPlugin, Kontact::Core> KMailPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kmailplugin,
                            KMailPluginFactory( "kontact_kmailplugin" ) )

KMailPlugin::KMailPlugin(Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "kmail" ),
    m_instance( 0 )
{
  setComponentData( KMailPluginFactory::componentData() );

  KAction *action  = new KAction(KIcon("mail_new"), i18n("New Message..."), this);
  actionCollection()->addAction("new_mail", action );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_M));
  connect(action, SIGNAL(triggered(bool)), SLOT( slotNewMail() ));
  insertNewAction(action);

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KMailUniqueAppHandler>(), this );
}

bool KMailPlugin::canDecodeMimeData( const QMimeData *mimeData )
{
  return ( ICalDrag::canDecode( mimeData ) ||
           VCalDrag::canDecode( mimeData ) ||
           KVCardDrag::canDecode( mimeData ) );
}

void KMailPlugin::processDropEvent( QDropEvent * de )
{
  kDebug() << k_funcinfo << endl;
  CalendarLocal cal( QString::fromLatin1("UTC") );
  KABC::Addressee::List list;
  const QMimeData *md = de->mimeData();

  if ( VCalDrag::fromMimeData( md, &cal ) || ICalDrag::fromMimeData( md, &cal ) ) {
    KTemporaryFile tmp;
    tmp.setPrefix("incidences-");
    tmp.setSuffix(".ics");
    tmp.setAutoRemove(false);
    tmp.open();
    cal.save( tmp.fileName() );
    openComposer( KUrl( tmp.fileName() ) );
  }
  else if ( KVCardDrag::fromMimeData( md, list ) ) {
    KABC::Addressee::List::Iterator it;
    QStringList to;
    for ( it = list.begin(); it != list.end(); ++it ) {
      to.append( ( *it ).fullEmail() );
    }
    openComposer( to.join(", ") );
  }

}

void KMailPlugin::openComposer( const KUrl& attach )
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_instance );
  if ( m_instance ) {
    if ( attach.isValid() )
      m_instance->newMessage( "", "", "", false, true, QString(), attach.path() );
    else
      m_instance->newMessage( "", "", "", false, true, QString(), QString() );
  }
}

void KMailPlugin::openComposer( const QString& to )
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( m_instance );
  if ( m_instance ) {
    m_instance->newMessage( to, "", "", false, true, QString(), QString() );
  }
}

void KMailPlugin::slotNewMail()
{
  openComposer( QString::null );
}

KMailPlugin::~KMailPlugin()
{
}

bool KMailPlugin::createDBUSInterface( const QString& serviceType )
{
  if ( serviceType == "DBUS/ResourceBackend/IMAP" ) {
    if ( part() )
      return true;
  }

  return false;
}

QString KMailPlugin::tipFile() const
{
  QString file = KStandardDirs::locate("data", "kmail/tips");
  return file;
}

KParts::ReadOnlyPart* KMailPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();
  if ( !part ) return 0;

  m_instance = new OrgKdeKmailKmailInterface( "org.kde.kmail", "/KMail", QDBusConnection::sessionBus() ); 

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
    KCmdLineArgs::addCmdLineOptions( kmail_options() );
}

int KMailUniqueAppHandler::newInstance()
{
    kDebug()<< k_funcinfo <<endl;
    // Ensure part is loaded
    (void)plugin()->part();
    org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    QDBusReply<bool> reply = kmail.handleCommandLine(false);
    
    if ( reply.isValid() ) {
        bool handled = reply;
        kDebug(5602) << k_funcinfo << "handled=" << handled << endl;
        if ( !handled ) // no args -> simply bring kmail plugin to front
            return Kontact::UniqueAppHandler::newInstance();
    }
    return 0;
}

bool KMailPlugin::queryClose() const {
   org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
   QDBusReply<bool> canClose = kmail.canQueryClose();
   return canClose;
}

#include "kmail_plugin.moc"
