/*  -*- mode: C++; c-file-style: "gnu" -*-

    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>

#include "core.h"
#include "summary.h"
#include "summarywidget.h"

#include <time.h>
#include "kmailinterface.h"
#include "kmail_folder_interface.h"
#define DBUS_KMAIL "org.kde.kmail"

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ),
//    DCOPObject( "MailSummary" ),
    mPlugin( plugin )
{
  QDBusConnection::sessionBus().registerObject("/MailSummary", this);
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QPixmap icon = KIconLoader::global()->loadIcon( "kontact_mail", K3Icon::Desktop,
                                                  K3Icon::SizeMedium );
  QWidget *header = createHeader(this, icon, i18n("New Messages"));
  mLayout = new QGridLayout();
  mLayout->setSpacing( 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);

  slotUnreadCountChanged();
  QDBusConnection::sessionBus().connect(QString(), "/KMail","org.kde.kmail.kmail", "unreadCountChanged", this, SLOT(slotUnreadCountChanged()));
}

void SummaryWidget::selectFolder( const QString& folder )
{
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
  QByteArray data;
  QDataStream arg( &data, QIODevice::WriteOnly );
  arg << folder;
#ifdef __GNUC__
#warning Port DCOP signal!
#endif
//  emitDCOPSignal( "kmailSelectFolder(QString)", data );
}

void SummaryWidget::updateSummary( bool )
{
  // check whether we need to update the message counts
  org::kde::kmail::kmail kmail( DBUS_KMAIL , "/KMail" , QDBusConnection::sessionBus());
  const int timeOfLastMessageCountChange = kmail.timeOfLastMessageCountChange();
  if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate )
    slotUnreadCountChanged();
}

void SummaryWidget::slotUnreadCountChanged()
{
  kDebug()<<" SummaryWidget::slotUnreadCountChanged\n";
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail" , QDBusConnection::sessionBus());
  QDBusReply<QStringList> reply = kmail.folderList();
  if ( reply.isValid() ) {
    QStringList folderList = reply;
    updateFolderList( folderList );
  }
  else {
    kDebug(5602) << "Calling kmail->KMailIface->folderList() via D-Bus failed."
                  << endl;
  }
  mTimeOfLastMessageCountUpdate = ::time( 0 );
}

void SummaryWidget::updateFolderList( const QStringList& folders )
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig _config( "kcmkmailsummaryrc" );
  KConfigGroup config(&_config, "General" );

  QStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) )
    activeFolders << "/Local/inbox";
  else
    activeFolders = config.readEntry( "ActiveFolders" , QStringList() );

  int counter = 0;
  QStringList::ConstIterator it;
  org::kde::kmail::kmail kmail( DBUS_KMAIL , "/KMail" , QDBusConnection::sessionBus());
  for ( it = folders.begin(); it != folders.end(); ++it ) {
    if ( activeFolders.contains( *it ) ) {
      QDBusReply<QDBusObjectPath> ref = kmail.getFolder( *it );
      if ( ref.isValid() )
        {
          QDBusObjectPath path = ref;
          OrgKdeKmailFolderInterface folderInterface(  DBUS_KMAIL, path.path(), QDBusConnection::sessionBus());
          QDBusReply<int> replyNumMsg = folderInterface.messages();
          const int numMsg = replyNumMsg;
          QDBusReply<int> replyUnreadNumMsg = folderInterface.unreadMessages();
          const int numUnreadMsg=replyUnreadNumMsg;

          if ( numUnreadMsg == 0 ) continue;

          QString folderPath;
          QDBusReply<QString> replyFolderPath;
          if ( config.readEntry( "ShowFullPath", true ) )
            replyFolderPath= folderInterface.displayPath();
          else
            replyFolderPath= folderInterface.displayName();
          folderPath = replyFolderPath;
          KUrlLabel *urlLabel = new KUrlLabel( *it, folderPath, this );
          urlLabel->installEventFilter( this );
          urlLabel->setAlignment( Qt::AlignLeft );
          urlLabel->show();
          connect( urlLabel, SIGNAL( leftClickedUrl( const KUrl&) ),
                   SLOT( selectFolder( const KUrl& ) ) );
          mLayout->addWidget( urlLabel, counter, 0 );
          mLabels.append( urlLabel );

          QLabel *label =
            new QLabel( i18nc("%1: number of unread messages "
                              "%2: total number of messages", "%1 / %2",
                              numUnreadMsg, numMsg ), this );
          label->setAlignment( Qt::AlignLeft );
          label->show();
          mLayout->addWidget( label, counter, 2 );
          mLabels.append( label );

          counter++;
        }
    }
  }

  if ( counter == 0 ) {
    QLabel *label = new QLabel( i18n( "No unread messages in your monitored folders" ), this );
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( label, 0, 0, 1, 3 );
    label->show();
    mLabels.append( label );
  }
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Open Folder: \"%1\"", label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
