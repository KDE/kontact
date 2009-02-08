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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "summarywidget.h"
#include "kmailinterface.h"
#include "kmail_folder_interface.h"

#include <kontactinterfaces/core.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kparts/part.h>

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QToolTip>
#include <QVBoxLayout>

#include <time.h>

#define DBUS_KMAIL "org.kde.kmail"

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ), mPlugin( plugin )
{
  QDBusConnection::sessionBus().registerObject( "/MailSummary", this );
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QWidget *header = createHeader( this, "view-pim-mail", i18n( "New Messages" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  slotUnreadCountChanged();
  QDBusConnection::sessionBus().connect(
    QString(), "/KMail","org.kde.kmail.kmail", "unreadCountChanged",
    this, SLOT(slotUnreadCountChanged()) );
}

void SummaryWidget::selectFolder( const QString &folder )
{
  if ( mPlugin->isRunningStandalone() ) {
    mPlugin->bringToForeground();
  } else {
    mPlugin->core()->selectPlugin( mPlugin );
  }

  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  kmail.selectFolder( folder );
}

void SummaryWidget::updateSummary( bool )
{
  // check whether we need to update the message counts
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  const int timeOfLastMessageCountChange = kmail.timeOfLastMessageCountChange();
  if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate ) {
    slotUnreadCountChanged();
  }
}

void SummaryWidget::slotUnreadCountChanged()
{
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  QDBusReply<QStringList> reply = kmail.folderList();
  if ( reply.isValid() ) {
    QStringList folderList = reply;
    updateFolderList( folderList );
  } else {
    kDebug() << "Calling kmail->KMailIface->folderList() via D-Bus failed.";
  }
  mTimeOfLastMessageCountUpdate = ::time( 0 );
}

void SummaryWidget::updateFolderList( const QStringList &folders )
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig _config( "kcmkmailsummaryrc" );
  KConfigGroup config( &_config, "General" );

  QStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) ) {
    activeFolders << "/Local/inbox";
  } else {
    activeFolders = config.readEntry( "ActiveFolders", QStringList() );
  }

  QString defName = "view-pim-mail";
  QLabel *label = 0;
  int counter = 0;
  QStringList::ConstIterator it;
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  for ( it = folders.begin(); it != folders.end(); ++it ) {
    if ( activeFolders.contains( *it ) ) {
      QDBusReply<QString> ref = kmail.getFolder( *it );
      if ( ref.isValid() && !ref.value().isEmpty() ) {
        OrgKdeKmailFolderInterface folderInterface(
          DBUS_KMAIL, "/Folder", QDBusConnection::sessionBus() );
        QDBusReply<int> replyNumMsg = folderInterface.messages();
        const int numMsg = replyNumMsg;
        QDBusReply<int> replyUnreadNumMsg = folderInterface.unreadMessages();
        const int numUnreadMsg=replyUnreadNumMsg;

        if ( numUnreadMsg == 0 ) {
          continue;
        }

        // folder icon
        QString name = folderInterface.unreadIconPath();
        if ( name.isEmpty() ) {
          name = defName;
        }
        label = new QLabel( this );
        label->setPixmap( KIconLoader::global()->loadIcon( name, KIconLoader::Small ) );
        label->setMaximumWidth( label->minimumSizeHint().width() );
        label->setAlignment( Qt::AlignVCenter );
        mLayout->addWidget( label, counter, 0 );
        mLabels.append( label );

        // folder path
        QString folderPath;
        QDBusReply<QString> replyFolderPath;
        if ( config.readEntry( "ShowFullPath", true ) ) {
          replyFolderPath= folderInterface.displayPath();
        } else {
          replyFolderPath= folderInterface.displayName();
        }
        folderPath = replyFolderPath;
        KUrlLabel *urlLabel = new KUrlLabel( *it, folderPath, this );
        urlLabel->installEventFilter( this );
        urlLabel->setAlignment( Qt::AlignLeft );
        urlLabel->setWordWrap( true );
        mLayout->addWidget( urlLabel, counter, 1 );
        mLabels.append( urlLabel );

        connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
                 SLOT(selectFolder(const QString&)) );

        // unread of total
        label = new QLabel( i18nc( "%1: number of unread messages "
                                   "%2: total number of messages",
                                   "%1 / %2", numUnreadMsg, numMsg ), this );
        label->setAlignment( Qt::AlignLeft );
        mLayout->addWidget( label, counter, 2 );
        mLabels.append( label );

        // tooltip
        urlLabel->setToolTip( i18n( "<qt><b>%1</b>"
                                    "<br/>Total: %2<br/>"
                                    "Unread: %3</qt>",
                                    folderPath,
                                    numMsg,
                                    numUnreadMsg ) );
        //TODO: put the folder size in the tooltip
        //so we need to add a folderSize() to the interface
        counter++;
      }
    }
  }

  if ( counter == 0 ) {
    label = new QLabel( i18n( "No unread messages in your monitored folders" ), this );
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( label );
    mLabels.append( label );
  }

  QList<QLabel*>::iterator lit;
  for ( lit = mLabels.begin(); lit != mLabels.end(); ++lit ) {
    (*lit)->show();
  }
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Open Folder: \"%1\"", label->text() ) );
    }  if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
