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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>
#include <qdragobject.h>

#include <kapplication.h>
#include <kabc/vcardconverter.h>
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <dcopclient.h>

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kvcarddrag.h>

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

  instance()->iconLoader()->addAppDir( "korganizer" );
  QPixmap pm = instance()->iconLoader()->loadIcon( "newtodo", KIcon::Toolbar );
  insertNewAction( new KAction( i18n( "New To-do..." ), pm,
                   0, this, SLOT( slotNewTodo() ), actionCollection(),
                   "new_todo" ) );

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

KPIM::Part *TodoPlugin::createPart()
{
  KPIM::Part *part = loadPart();

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
      QString uri = "kmail:" + QString::number( mail.serialNumber() ) + "/" +
                    mail.messageId();
      interface()->openTodoEditor( i18n("Mail: %1").arg( mail.subject() ), txt,
                                   uri );
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.")
                              .arg( event->format() ) );
}

#include "todoplugin.moc"
