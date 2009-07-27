/*
    This file is part of Kontact.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qwidget.h>
#include <qdragobject.h>
#include <qfile.h>

#include <kapplication.h>
#include <kabc/vcardconverter.h>
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <ktempfile.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icaldrag.h>

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/kpimprefs.h>

#include "core.h"

#include "todoplugin.h"
#include "todosummarywidget.h"
#include "korg_uniqueapp.h"

typedef KGenericFactory< TodoPlugin, Kontact::Core > TodoPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_todoplugin,
                            TodoPluginFactory( "kontact_todoplugin" ) )

TodoPlugin::TodoPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{
  setInstance( TodoPluginFactory::instance() );
  instance()->iconLoader()->addAppDir("kdepim");

  insertNewAction( new KAction( i18n( "New To-do..." ), "newtodo",
                   CTRL+SHIFT+Key_T, this, SLOT( slotNewTodo() ), actionCollection(),
                   "new_todo" ) );

  insertSyncAction( new KAction( i18n( "Synchronize To-do List" ), "reload",
                   0, this, SLOT( slotSyncTodos() ), actionCollection(),
                   "todo_sync" ) );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

TodoPlugin::~TodoPlugin()
{
}

Kontact::Summary *TodoPlugin::createSummaryWidget( QWidget *parent )
{
  return new TodoSummaryWidget( this, parent );
}

KParts::ReadOnlyPart *TodoPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part )
    return 0;

  dcopClient(); // ensure that we register to DCOP as "korganizer"
  mIface = new KCalendarIface_stub( dcopClient(), "kontact", "CalendarIface" );

  return part;
}

void TodoPlugin::select()
{
  interface()->showTodoView();
}

QStringList TodoPlugin::invisibleToolbarActions() const
{
  QStringList invisible;
  invisible += "new_event";
  invisible += "new_todo";
  invisible += "new_journal";

  invisible += "view_day";
  invisible += "view_list";
  invisible += "view_workweek";
  invisible += "view_week";
  invisible += "view_nextx";
  invisible += "view_month";
  invisible += "view_journal";
  return invisible;
}

KCalendarIface_stub *TodoPlugin::interface()
{
  if ( !mIface ) {
    part();
  }
  Q_ASSERT( mIface );
  return mIface;
}

void TodoPlugin::slotNewTodo()
{
  interface()->openTodoEditor( "" );
}

void TodoPlugin::slotSyncTodos()
{
  DCOPRef ref( "kmail", "KMailICalIface" );
  ref.send( "triggerSync", QString("Todo") );
}

bool TodoPlugin::createDCOPInterface( const QString& serviceType )
{
  kdDebug(5602) << k_funcinfo << serviceType << endl;
  if ( serviceType == "DCOP/Organizer" || serviceType == "DCOP/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool TodoPlugin::canDecodeDrag( QMimeSource *mimeSource )
{
  return QTextDrag::canDecode( mimeSource ) ||
         KPIM::MailListDrag::canDecode( mimeSource );
}

bool TodoPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

void TodoPlugin::processDropEvent( QDropEvent *event )
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

    interface()->openTodoEditor( i18n( "Meeting" ), QString::null, QString::null,
                                 attendees );
    return;
  }

  if ( KCal::ICalDrag::canDecode( event) ) {
    KCal::CalendarLocal cal( KPimPrefs::timezone() );
    if ( KCal::ICalDrag::decode( event, &cal ) ) {
      KCal::Incidence::List incidences = cal.incidences();
      if ( !incidences.isEmpty() ) {
        event->accept();
        KCal::Incidence *i = incidences.first();
        QString summary;
        if ( dynamic_cast<KCal::Journal*>( i ) )
          summary = i18n( "Note: %1" ).arg( i->summary() );
        else
          summary = i->summary();
        interface()->openTodoEditor( summary, i->description(), QString() );
        return;
      }
      // else fall through to text decoding
    }
  }

  if ( QTextDrag::decode( event, text ) ) {
    interface()->openTodoEditor( text );
    return;
  }

  KPIM::MailList mails;
  if ( KPIM::MailListDrag::decode( event, mails ) ) {
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n("Drops of multiple mails are not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();
      QString txt = i18n("From: %1\nTo: %2\nSubject: %3").arg( mail.from() )
                    .arg( mail.to() ).arg( mail.subject() );

      KTempFile tf;
      tf.setAutoDelete( true );
      QString uri = "kmail:" + QString::number( mail.serialNumber() ) + "/" +
                    mail.messageId();
      tf.file()->writeBlock( event->encodedData( "message/rfc822" ) );
      tf.close();
      interface()->openTodoEditor( i18n("Mail: %1").arg( mail.subject() ),
                                   txt, uri, tf.name(), QStringList(), "message/rfc822", false );
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.")
                              .arg( event->format() ) );
}

#include "todoplugin.moc"
