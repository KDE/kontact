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

#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <Akonadi/ChangeRecorder>

#include <KConfigGroup>
#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <KUrlLabel>

#include <QEvent>
#include <QGridLayout>
#include <QVBoxLayout>

#include <ctime>

SummaryWidget::SummaryWidget( KontactInterface::Plugin *plugin, QWidget *parent )
  : KontactInterface::Summary( parent ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QWidget *header = createHeader( this, "view-pim-mail", i18n( "New Messages" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  // Create a new change recorder.
  mChangeRecorder = new Akonadi::ChangeRecorder( this );
  mChangeRecorder->setMimeTypeMonitored( "Message/rfc822" );

  slotUnreadCountChanged();
}

void SummaryWidget::selectFolder( const QString &folder )
{
  if ( mPlugin->isRunningStandalone() ) {
    mPlugin->bringToForeground();
  } else {
    mPlugin->core()->selectPlugin( mPlugin );
  }

#if 0 // TODO: Port to Akonadi
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  kmail.selectFolder( folder );
#else
  kWarning() << "Port to Akonadi";
#endif
}

void SummaryWidget::updateSummary( bool )
{
#if 0 // TODO: Port to Akonadi
  // check whether we need to update the message counts
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  if ( kmail.isValid() ) {
    QDBusReply<int> timeOfLastMessageCountChange = kmail.timeOfLastMessageCountChange();
    if ( timeOfLastMessageCountChange.isValid() ) {
      if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate ) {
        slotUnreadCountChanged();
      }
    }
  }
#else
  kWarning() << "Port to Akonadi";
#endif
}

void SummaryWidget::slotUnreadCountChanged()
{
#if 0 // TODO: Port to Akonadi
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  QDBusReply<QStringList> reply = kmail.folderList();
  if ( reply.isValid() ) {
    QStringList folderList = reply;
    updateFolderList( folderList );
  } else {
    kWarning() << "Calling kmail->KMailIface->folderList() via D-Bus failed.";
  }
  mTimeOfLastMessageCountUpdate = ::time( 0 );
#else
  kWarning() << "Port to Akonadi";
#endif
}

void SummaryWidget::updateFolderList( const QStringList &folders )
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig _config( "kcmkmailsummaryrc" );
  KConfigGroup config( &_config, "General" );
  QLabel *label = 0;
  int counter = 0;

#if 0 // TODO port to Akonadi
  QStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) ) {
    activeFolders << "/Local/inbox";
  } else {
    activeFolders = config.readEntry( "ActiveFolders", QStringList() );
  }

  QString defName = "view-pim-mail";
  QStringList::ConstIterator it;
  org::kde::kmail::kmail kmail( DBUS_KMAIL, "/KMail", QDBusConnection::sessionBus() );
  if ( kmail.isValid() ) {
    for ( it = folders.begin(); it != folders.end(); ++it ) {
      if ( activeFolders.contains( *it ) ) {
        QDBusReply<QString> ref = kmail.getFolder( *it );
        if ( ref.isValid() && !ref.value().isEmpty() ) {
          OrgKdeKmailFolderInterface folderInterface(
            DBUS_KMAIL, "/Folder", QDBusConnection::sessionBus() );
          if ( !folderInterface.isValid() ) {
            continue;
          }

          QDBusReply<int> replyNumMsg = folderInterface.messages();
          if ( !replyNumMsg.isValid() ) {
            continue;
          }

          const int numMsg = replyNumMsg;
          QDBusReply<int> replyUnreadNumMsg = folderInterface.unreadMessages();
          if ( !replyUnreadNumMsg.isValid() ) {
            continue;
          }

          const int numUnreadMsg = replyUnreadNumMsg;
          if ( numUnreadMsg == 0 ) {
            continue;
          }

          // folder icon
          QDBusReply<QString> dbusName = folderInterface.unreadIconPath();
          QString name;
          if ( dbusName.isValid() ) {
            name = dbusName;
            if ( name.isEmpty() ) {
              name = defName;
            }
          } else {
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
          if ( replyFolderPath.isValid() ) {
            folderPath = replyFolderPath;
          }

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
  }
#else
        kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

  if ( counter == 0 ) {
    label = new QLabel( i18n( "No unread messages in your monitored folders" ), this );
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( label, 0, 0 );
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

  return KontactInterface::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
