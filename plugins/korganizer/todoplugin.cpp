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
#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <dcopclient.h>

#include <libkdepim/maillistdrag.h>

#include "core.h"

#include "todoplugin.h"

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
  insertNewAction( new KAction( i18n( "New Todo" ), pm,
                   0, this, SLOT( slotNewTodo() ), actionCollection(),
                   "new_todo" ) );
}

TodoPlugin::~TodoPlugin()
{
}

Kontact::Summary *TodoPlugin::createSummaryWidget( QWidget * /*parent*/ )
{
  return 0;
}

KParts::Part *TodoPlugin::createPart()
{
  KParts::Part *part = loadPart();

  if ( !part )
    return 0;

  KAction *action = part->actionCollection()->action( "new_todo" );
  if ( action )
    part->actionCollection()->take( action );

  dcopClient(); // ensure that we register to DCOP as "korganizer"
  mIface = new KCalendarIface_stub( dcopClient(), "kontact", "CalendarIface" );

  return part;
}

void TodoPlugin::select()
{
  interface()->showTodoView();
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
  DCOPClient* dc = kapp->dcopClient();

  return (dc->isApplicationRegistered("korganizer")) &&
         (!dc->remoteObjects("kontact").contains("CalendarIface"));
}

void TodoPlugin::processDropEvent( QDropEvent *event )
{
  QString text;
  if ( QTextDrag::decode( event, text ) ) {
    interface()->openTodoEditor( text );
    return;
  }

  KPIM::MailList mails;
  if ( KPIM::MailListDrag::decode( event, mails ) ) {
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n("Drops of multiple mails aren't supported." ) );
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

  KMessageBox::sorry( core(), i18n("Can't handle drop events of type '%1'.")
                              .arg( event->format() ) );
}

#include "todoplugin.moc"
