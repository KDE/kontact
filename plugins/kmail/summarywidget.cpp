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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>

#include <dcopref.h>
#include <kapplication.h>
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

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( QCString("MailSummary") ),
    mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_mail", KIcon::Desktop,
                                                  KIcon::SizeMedium );
  QWidget *header = createHeader(this, icon, i18n("New Messages"));
  mLayout = new QGridLayout( 1, 3, 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);

  slotUnreadCountChanged();
  connectDCOPSignal( 0, 0, "unreadCountChanged()", "slotUnreadCountChanged()",
                     false );
}

void SummaryWidget::selectFolder( const QString& folder )
{
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
  QByteArray data;
  QDataStream arg( data, IO_WriteOnly );
  arg << folder;
  emitDCOPSignal( "kmailSelectFolder(QString)", data );
}

void SummaryWidget::updateSummary( bool )
{
  // check whether we need to update the message counts
  DCOPRef kmail( "kmail", "KMailIface" );
  const int timeOfLastMessageCountChange =
    kmail.call( "timeOfLastMessageCountChange()" );
  if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate )
    slotUnreadCountChanged();
}

void SummaryWidget::slotUnreadCountChanged()
{
  DCOPRef kmail( "kmail", "KMailIface" );
  DCOPReply reply = kmail.call( "folderList" );
  if ( reply.isValid() ) {
    QStringList folderList = reply;
    updateFolderList( folderList );
  }
  else {
    kdDebug(5602) << "Calling kmail->KMailIface->folderList() via DCOP failed."
                  << endl;
  }
  mTimeOfLastMessageCountUpdate = ::time( 0 );
}

void SummaryWidget::updateFolderList( const QStringList& folders )
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) )
    activeFolders << "/Local/inbox";
  else
    activeFolders = config.readListEntry( "ActiveFolders" );

  bool showFullPath = config.readBoolEntry( "ShowFullPath", false );

  int counter = 0;
  QStringList::ConstIterator it;
  DCOPRef kmail( "kmail", "KMailIface" );
  for ( it = folders.begin(); it != folders.end() && counter < 9; ++it ) {
    if ( activeFolders.contains( *it ) ) {
      DCOPRef folderRef = kmail.call( "getFolder(QString)", *it );
      const int numMsg = folderRef.call( "messages()" );
      const int numUnreadMsg = folderRef.call( "unreadMessages()" );

      if ( numUnreadMsg == 0 ) continue;

      QString folderPath;
      if ( showFullPath )
        folderRef.call( "displayPath()" ).get( folderPath );
      else
        folderRef.call( "displayName()" ).get( folderPath );

      KURLLabel *urlLabel = new KURLLabel( *it, folderPath, this );
      urlLabel->setAlignment( AlignLeft );
      urlLabel->show();
      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               SLOT( selectFolder( const QString& ) ) );
      mLayout->addWidget( urlLabel, counter, 0 );
      mLabels.append( urlLabel );

      QLabel *label =
        new QLabel( QString( i18n("%1: number of unread messages "
                                  "%2: total number of messages", "%1 / %2") )
                    .arg( numUnreadMsg ).arg( numMsg ), this );
      label->setAlignment( AlignLeft );
      label->show();
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      counter++;
    }
  }

  if ( counter == 0 ) {
    QLabel *label = new QLabel( i18n( "No unread messages" ), this );
    label->show();
    mLayout->addMultiCellWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );
  }
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
