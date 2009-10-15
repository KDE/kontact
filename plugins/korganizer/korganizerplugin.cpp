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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "korganizerplugin.h"
#include "apptsummarywidget.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"

#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/maillistdrag.h>

#include <KCal/CalendarLocal>
#include <KCal/ICalDrag>

#include <KontactInterface/Core>

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>
#include <KSystemTimeZone>
#include <KTemporaryFile>

#include <QDropEvent>

EXPORT_KONTACT_PLUGIN( KOrganizerPlugin, korganizer )

KOrganizerPlugin::KOrganizerPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "korganizer", "calendar" ), mIface( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );
  KIconLoader::global()->addAppDir( "korganizer" );
  KIconLoader::global()->addAppDir( "kdepim" );

  KAction *action  =
    new KAction( KIcon( "appointment-new" ),
                 i18nc( "@action:inmenu", "New Event..." ), this );
  actionCollection()->addAction( "new_event", action );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT+Qt::Key_E ) );
  action->setHelpText(
    i18nc( "@info:status", "Create a new event" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can create a new event item." ) );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotNewEvent()) );
  insertNewAction( action );

  KAction *syncAction =
    new KAction( KIcon( "view-refresh" ),
                 i18nc( "@action:inmenu", "Sync Calendar" ), this );
  actionCollection()->addAction( "korganizer_sync", syncAction );
  syncAction->setHelpText(
    i18nc( "@info:status", "Synchronize groupware calendar" ) );
  syncAction->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose this option to synchronize your groupware events." ) );
  connect( syncAction, SIGNAL(triggered(bool)), SLOT(slotSyncEvents()) );
  insertSyncAction( syncAction );

  mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
    new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

KontactInterface::Summary *KOrganizerPlugin::createSummaryWidget( QWidget *parent )
{
  return new ApptSummaryWidget( this, parent );
}

KParts::ReadOnlyPart *KOrganizerPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part ) {
    return 0;
  }

  mIface = new OrgKdeKorganizerCalendarInterface(
    "org.kde.korganizer", "/Calendar", QDBusConnection::sessionBus(), this );

  return part;
}

QString KOrganizerPlugin::tipFile() const
{
  QString file = KStandardDirs::locate( "data", "korganizer/tips" );
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
  QDBusMessage message =
      QDBusMessage::createMethodCall( "org.kde.kmail", "/Groupware",
                                      "org.kde.kmail.groupware",
                                      "triggerSync" );
  message << QString( "Calendar" );
  QDBusConnection::sessionBus().send( message );
}

bool KOrganizerPlugin::createDBUSInterface( const QString &serviceType )
{
  if ( serviceType == "DBUS/Organizer" || serviceType == "DBUS/Calendar" ) {
    if ( part() ) {
      return true;
    }
  }
  return false;
}

bool KOrganizerPlugin::isRunningStandalone() const
{
  return mUniqueAppWatcher->isRunningStandalone();
}

bool KOrganizerPlugin::canDecodeMimeData( const QMimeData *mimeData ) const
{
  return mimeData->hasText() ||
    KPIM::MailList::canDecode( mimeData ) ||
    KPIM::KVCardDrag::canDecode( mimeData );
}

void KOrganizerPlugin::processDropEvent( QDropEvent *event )
{
  const QMimeData *md = event->mimeData();
  if ( KPIM::KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List contacts;

    KPIM::KVCardDrag::fromMimeData( md, contacts );

    KABC::Addressee::List::Iterator it;

    QStringList attendees;
    for ( it = contacts.begin(); it != contacts.end(); ++it ) {
      QString email = (*it).fullEmail();
      if ( email.isEmpty() ) {
        attendees.append( (*it).realName() + "<>" );
      } else {
        attendees.append( email );
      }
    }

    interface()->openEventEditor( i18nc( "@item", "Meeting" ),
                                  QString(), QStringList(), attendees );
    return;
  }

  if ( KCal::ICalDrag::canDecode( event->mimeData() ) ) {
      KCal::CalendarLocal cal( KSystemTimeZones::local() );
      if ( KCal::ICalDrag::fromMimeData( event->mimeData(), &cal ) ) {
          KCal::Incidence::List incidences = cal.incidences();
          Q_ASSERT( incidences.count() );
          if ( !incidences.isEmpty() ) {
              event->accept();
              KCal::Incidence *i = incidences.first();
              QString summary;
              if ( dynamic_cast<KCal::Journal*>( i ) ) {
                summary = i18nc( "@item", "Note: %1", i->summary() );
              } else {
                summary = i->summary();
              }
              interface()->openEventEditor( summary, i->description(), QStringList() );
              return;
          }
      // else fall through to text decoding
      }
  }

  if ( md->hasText() ) {
    QString text = md->text();
    kDebug() << "DROP:" << text;
    interface()->openEventEditor( text );
    return;
  }

  if ( KPIM::MailList::canDecode( md ) ) {
    KPIM::MailList mails = KPIM::MailList::fromMimeData( md );
    event->accept();
    if ( mails.count() != 1 ) {
      KMessageBox::sorry(
        core(),
        i18nc( "@info", "Dropping multiple mails is not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();
      QString txt = i18nc( "@item", "From: %1\nTo: %2\nSubject: %3",
                          mail.from(), mail.to(), mail.subject() );

      KTemporaryFile tf;
      tf.setAutoRemove( true );
      tf.open();
      QString uri = KDEPIMPROTOCOL_EMAIL + QString::number( mail.serialNumber() );
      tf.write( event->encodedData( "message/rfc822" ) );
      interface()->openEventEditor(
        i18nc( "@item", "Mail: %1", mail.subject() ), txt,
        uri, tf.fileName(), QStringList(), "message/rfc822" );
      tf.close();
    }
    return;
  }

  kWarning() << QString( "Cannot handle drop events of type '%1'." ).arg( event->format() );
}

#include "korganizerplugin.moc"
