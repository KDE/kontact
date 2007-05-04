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

#include <qcursor.h>
#include <qfile.h>
#include <qwidget.h>
#include <qdragobject.h>

#include <kapplication.h>
#include <kabc/vcardconverter.h>
#include <kaction.h>
#include <dcopref.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <dcopclient.h>

#include <libkmime/kmime_message.h>

#include <libkdepim/kvcarddrag.h>
#include <libkdepim/maillistdrag.h>

#include "core.h"
#include "summarywidget.h"
#include "korganizerplugin.h"
#include "korg_uniqueapp.h"

#define DROP_CANCEL 0
#define DROP_URI 1
#define DROP_INLINE_FULL 2
#define DROP_INLINE_BODY 3

typedef KGenericFactory< KOrganizerPlugin, Kontact::Core > KOrganizerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_korganizerplugin,
                            KOrganizerPluginFactory( "kontact_korganizerplugin" ) )

KOrganizerPlugin::KOrganizerPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{

  setInstance( KOrganizerPluginFactory::instance() );
  instance()->iconLoader()->addAppDir("kdepim");

  insertNewAction( new KAction( i18n( "New Event..." ), "newappointment",
                   CTRL+SHIFT+Key_E, this, SLOT( slotNewEvent() ), actionCollection(),
                   "new_event" ) );

  insertSyncAction( new KAction( i18n( "Synchronize Calendar" ), "reload",
                   0, this, SLOT( slotSyncEvents() ), actionCollection(),
                   "korganizer_sync" ) );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

Kontact::Summary *KOrganizerPlugin::createSummaryWidget( QWidget *parent )
{
  return new SummaryWidget( this, parent );
}

KParts::ReadOnlyPart *KOrganizerPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part )
    return 0;

  mIface = new KCalendarIface_stub( dcopClient(), "kontact", "CalendarIface" );

  return part;
}

QString KOrganizerPlugin::tipFile() const
{
  QString file = ::locate("data", "korganizer/tips");
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

KCalendarIface_stub *KOrganizerPlugin::interface()
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
  DCOPRef ref( "kmail", "KMailICalIface" );
  ref.send( "triggerSync", QString("Calendar") );
}

bool KOrganizerPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool KOrganizerPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

bool KOrganizerPlugin::canDecodeDrag( QMimeSource *mimeSource )
{
  return QTextDrag::canDecode( mimeSource ) ||
         KPIM::MailListDrag::canDecode( mimeSource );
}

void KOrganizerPlugin::processDropEvent( QDropEvent *event )
{
  QString text;

  KABC::VCardConverter converter;
  if ( KVCardDrag::canDecode( event ) && KVCardDrag::decode( event, text ) ) {
    KABC::Addressee::List contacts = converter.parseVCards( text );
    KABC::Addressee::List::Iterator it;

    QStringList attendees;
    for ( it = contacts.begin(); it != contacts.end(); ++it ) {
      QString email = (*it).fullEmail();
      if ( email.isEmpty() )
        attendees.append( (*it).realName() + "<>" );
      else
        attendees.append( email );
    }

    interface()->openEventEditor( i18n( "Meeting" ), QString::null, QString::null,
                                  attendees );
    return;
  }

  if ( QTextDrag::decode( event, text ) ) {
    kdDebug(5602) << "DROP:" << text << endl;
    interface()->openEventEditor( text );
    return;
  }

  KPIM::MailList mails;
  if ( KPIM::MailListDrag::decode( event, mails ) ) {
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n("Drops of multiple mails are not supported." ) );
    } else {
      KPopupMenu *menu = new KPopupMenu( 0 );
      menu->insertItem( i18n("Attach as &link"), DROP_URI );
      menu->insertItem( i18n("Attach &inline"), DROP_INLINE_FULL );
      menu->insertItem( i18n("Attach inline &without attachments"), DROP_INLINE_BODY );
      menu->insertSeparator();
      menu->insertItem( SmallIcon("cancel"), i18n("C&ancel"), DROP_CANCEL );
      int action = menu->exec( QCursor::pos(), 0 );
      delete menu;

      if ( action == DROP_CANCEL )
        return;

      KPIM::MailSummary mail = mails.first();
      QString txt = i18n("From: %1\nTo: %2\nSubject: %3").arg( mail.from() )
                    .arg( mail.to() ).arg( mail.subject() );

      QString uri;
      KTempFile tf;
      tf.setAutoDelete( true );
      if ( action == DROP_URI )
        uri = QString::fromLatin1("kmail:") + QString::number( mail.serialNumber() );
      else if ( action == DROP_INLINE_FULL ) {
        tf.file()->writeBlock( event->encodedData( "message/rfc822" ) );
        tf.close();
        uri = tf.name();
      } else if ( action == DROP_INLINE_BODY ) {
        KMime::Message *msg = new KMime::Message();
        msg->setContent( QCString( event->encodedData( "message/rfc822" ) ) );
        QCString head = msg->head();
        msg->parse();
        if ( msg == msg->textContent() || msg->textContent() == 0 ) { // no attachments
          tf.file()->writeBlock( event->encodedData( "message/rfc822" ) );
        } else {
          if ( KMessageBox::warningContinueCancel( 0,
               i18n("Removing attachments from an email might invalidate its signature."),
               i18n("Remove Attachments"), KStdGuiItem::cont(), "BodyOnlyInlineAttachment" )
               != KMessageBox::Continue )
            return;
          // due to kmime shortcomings in KDE3, we need to assemble the result manually
          int begin = 0;
          int end = head.find( '\n' );
          bool skipFolded = false;
          while ( end >= 0 && end > begin ) {
            if ( head.find( "Content-Type:", begin, false ) != begin &&
                 head.find( "Content-Transfer-Encoding:", begin, false ) != begin &&
                 !(skipFolded && (head[begin] == ' ' || head[end] == '\t')) ) {
              QCString line = head.mid( begin, end - begin );
              tf.file()->writeBlock( line.data(), line.length() );
              tf.file()->writeBlock( "\n", 1 );
              skipFolded = false;
            } else {
              skipFolded = true;
            }

            begin = end + 1;
            end = head.find( '\n', begin );
            if ( end < 0 && begin < (int)head.length() )
              end = head.length() - 1;
          }
          QCString cte = msg->textContent()->contentTransferEncoding()->as7BitString();
          if ( !cte.stripWhiteSpace().isEmpty() ) {
            tf.file()->writeBlock( cte.data(), cte.length() );
            tf.file()->writeBlock( "\n", 1 );
          }
          QCString ct = msg->textContent()->contentType()->as7BitString();
          if ( !ct.stripWhiteSpace().isEmpty() )
            tf.file()->writeBlock( ct.data(), ct.length() );
          tf.file()->writeBlock( "\n", 1 );
          tf.file()->writeBlock( msg->textContent()->body() );
        }
        tf.close();
        uri = tf.name();
        delete msg;
      } else {
        kdFatal() << k_funcinfo << "Unknown drop type" << endl;
      }
      interface()->openEventEditor( i18n("Mail: %1").arg( mail.subject() ), txt,
                                    uri, QStringList(), "message/rfc822",
                                    action != DROP_URI );
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.")
                              .arg( event->format() ) );
}

#include "korganizerplugin.moc"
