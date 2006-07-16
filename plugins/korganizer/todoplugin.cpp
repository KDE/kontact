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

#include <QWidget>
#include <q3dragobject.h>
#include <QDropEvent>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kvcarddrag.h>

#include "core.h"

#include "todoplugin.h"
#include "todosummarywidget.h"
#include "korg_uniqueapp.h"
#include "calendarinterface.h"

typedef KGenericFactory< TodoPlugin, Kontact::Core > TodoPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_todoplugin,
                            TodoPluginFactory( "kontact_todoplugin" ) )

TodoPlugin::TodoPlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{
  setInstance( TodoPluginFactory::instance() );
  instance()->iconLoader()->addAppDir("korganizer");

  insertNewAction( new KAction( i18n( "New To-do..." ), "newtodo",
                   Qt::CTRL+Qt::SHIFT+Qt::Key_T, this, SLOT( slotNewTodo() ), actionCollection(), "new_todo" ) );

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

  #warning "Once we have a running korganizer, make sure that this dbus call really does what it should! Where is it needed, anyway?"
  mIface = new OrgKdeKorganizerCalendarInterface( "org.kde.Korganizer.Calendar", "/Calendar", QDBus::sessionBus(), this );

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

OrgKdeKorganizerCalendarInterface *TodoPlugin::interface()
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

bool TodoPlugin::createDBUSInterface( const QString& serviceType )
{
  kDebug(5602) << k_funcinfo << serviceType << endl;
  #warning "What is this needed for, and do we still need it with DBUS?"
  if ( serviceType == "DBUS/Organizer" || serviceType == "DBUS/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool TodoPlugin::canDecodeDrag( QMimeSource *mimeSource )
{
  return Q3TextDrag::canDecode( mimeSource ) ||
         KPIM::MailListDrag::canDecode( mimeSource );
}

bool TodoPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

void TodoPlugin::processDropEvent( QDropEvent *event )
{
  QString text;

  if ( KVCardDrag::canDecode( event ) ) {
    KABC::Addressee::List contacts;

    KVCardDrag::decode( event, contacts );

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

  if ( Q3TextDrag::decode( event, text ) ) {
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
      QString txt = i18n("From: %1\nTo: %2\nSubject: %3", mail.from() ,
                      mail.to(), mail.subject() );
      QString uri = KDEPIMPROTOCOL_EMAIL +
                    QString::number( mail.serialNumber() ) + '/' +
                    mail.messageId();
      interface()->openTodoEditor( i18n("Mail: %1", mail.subject() ), txt,
                                   uri );
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.",
                                event->format() ) );
}

#include "todoplugin.moc"
