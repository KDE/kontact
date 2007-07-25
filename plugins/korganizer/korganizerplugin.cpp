/*
    This file is part of Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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
#include <QtDBus/QtDBus>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kicon.h>
#include <ktemporaryfile.h>
#include <libkdepim/kdepimprotocols.h>
#include <kmime/kmime_message.h>

#include <libkdepim/kvcarddrag.h>
#include <libkdepim/maillistdrag.h>

#include "core.h"
#include "apptsummarywidget.h"
#include "korganizerplugin.h"
#include "korg_uniqueapp.h"
#include "calendarinterface.h"

typedef KGenericFactory< KOrganizerPlugin, Kontact::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_korganizerplugin,
                            KOrganizerPluginFactory( "kontact_korganizerplugin" ) )

KOrganizerPlugin::KOrganizerPlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{
  setComponentData( KOrganizerPluginFactory::componentData() );
  KIconLoader::global()->addAppDir("korganizer");

  KAction *action  = new KAction(KIcon("appointment"), i18n("New Event..."), this);
  actionCollection()->addAction("new_event", action );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_E));
  connect(action, SIGNAL(triggered(bool)), SLOT( slotNewEvent() ));
  insertNewAction(action);

  KAction* syncAction = new KAction(KIcon("view-refresh"), i18n( "Synchronize Calendar" ), this);
  actionCollection()->addAction("korganizer_sync", syncAction);
  connect(action, SIGNAL(triggered(bool)), SLOT( slotSyncEvents()));
  insertSyncAction( syncAction );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

Kontact::Summary *KOrganizerPlugin::createSummaryWidget( QWidget *parent )
{
  return new ApptSummaryWidget( this, parent );
}

KParts::ReadOnlyPart *KOrganizerPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part )
    return 0;

#ifdef __GNUC__
  #warning "Once we have a running korganizer, make sure that this dbus call really does what it should! Where is it needed, anyway?"
#endif
  mIface = new OrgKdeKorganizerCalendarInterface( "org.kde.korganizer", "/Calendar", QDBusConnection::sessionBus(), this );

  return part;
}

QString KOrganizerPlugin::tipFile() const
{
  QString file = KStandardDirs::locate("data", "korganizer/tips");
  return file;
}

QStringList KOrganizerPlugin::invisibleToolbarActions() const
{
  QStringList invisible;
  invisible += "new_event";
  invisible += "new_todo";
  invisible += "new_journal";

  invisible += "view_todo";
  invisible += "view_journal";
  return invisible;
}

void KOrganizerPlugin::select()
{
  interface()->showEventView();
}

OrgKdeKorganizerCalendarInterface *KOrganizerPlugin::interface()
{
  if ( !mIface ) {
    part();
  }
  Q_ASSERT( mIface );
  return mIface;
}

void KOrganizerPlugin::slotNewEvent()
{
  interface()->openEventEditor( "" );
}

void KOrganizerPlugin::slotSyncEvents()
{
  QDBusMessage message = QDBusMessage::createSignal( "/Groupware", "org.kde.kmail", "triggerSync(QString)");
  message << QString( "Calendar" );
  QDBusConnection::sessionBus().send( message );
}

bool KOrganizerPlugin::createDBUSInterface( const QString& serviceType )
{
  kDebug(5602) << k_funcinfo << serviceType << endl;
#ifdef __GNUC__
  #warning "What is this needed for, and do we still need it with DBUS?"
#endif
  if ( serviceType == "DBUS/Organizer" || serviceType == "DBUS/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool KOrganizerPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

bool KOrganizerPlugin::canDecodeMimeData( const QMimeData *mimeData )
{
  return mimeData->hasText() || KPIM::MailList::canDecode( mimeData );
}

void KOrganizerPlugin::processDropEvent( QDropEvent *event )
{
  const QMimeData *md = event->mimeData();
  if ( KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List contacts;

    KVCardDrag::fromMimeData( md, contacts );

    KABC::Addressee::List::Iterator it;

    QStringList attendees;
    for ( it = contacts.begin(); it != contacts.end(); ++it ) {
      QString email = (*it).fullEmail();
      if ( email.isEmpty() )
        attendees.append( (*it).realName() + "<>" );
      else
        attendees.append( email );
    }

    interface()->openEventEditor( i18n( "Meeting" ), QString::null, QStringList(),
                                  attendees );
    return;
  }

  if ( md->hasText() ) {
    QString text = md->text();
    kDebug(5602) << "DROP:" << text << endl;
    interface()->openEventEditor( text );
    return;
  }


  if ( KPIM::MailList::canDecode( md ) ) {
    KPIM::MailList mails = KPIM::MailList::fromMimeData( md );
    event->accept();
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n("Drops of multiple mails are not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();
      QString txt = i18n("From: %1\nTo: %2\nSubject: %3", mail.from() ,
                      mail.to(), mail.subject() );

      KTemporaryFile tf;
      tf.setAutoRemove( true );
      QString uri = KDEPIMPROTOCOL_EMAIL + QString::number( mail.serialNumber() );
      tf.write( event->encodedData( "message/rfc822" ) );
      interface()->openEventEditor( i18n("Mail: %1", mail.subject() ), txt,
                                    uri, tf.name(), QStringList(), "message/rfc822" );
      tf.close();
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.",
                                event->format() ) );
}

#include "korganizerplugin.moc"
