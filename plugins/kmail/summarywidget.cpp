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

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( QCString("MailSummary") ),
    mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kmail", KIcon::Desktop, KIcon::SizeMedium);
  QWidget *header = createHeader(this, icon, i18n("New Messages"));
  mLayout = new QGridLayout( 1, 3, 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);
  mainLayout->addStretch();

  slotUnreadCountChanged();
  connectDCOPSignal( 0, 0, "unreadCountChanged()", "slotUnreadCountChanged()",
                     false );
}


void SummaryWidget::raisePart()
{

  // FIXME: select specific folder when 'selectFolder' dcop call is implemented
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
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
    activeFolders << "/inbox";
  else
    activeFolders = config.readListEntry( "ActiveFolders" );

  bool showFullPath = config.readBoolEntry( "ShowFullPath", false );

  int counter = 0;
  QStringList::ConstIterator it;
  DCOPRef kmail( "kmail", "KMailIface" );
  for ( it = folders.begin(); it != folders.end() && counter < 9; ++it ) {
    DCOPReply reply = kmail.call( "getFolder", *it );
    if ( reply.isValid() ) {
      DCOPRef folderRef = reply;
      int numUnreadMsg = -1;
      DCOPReply dcopReply = folderRef.call( "unreadMessages" );
      if ( dcopReply.isValid() ) {
        numUnreadMsg = dcopReply;
      }
      else {
        kdDebug(5602) << "Calling folderRef->unreadMessages() via DCOP failed."
                      << endl;
      }

      if ( activeFolders.contains( *it ) ) {
        QString folderPath = *it;
        if ( !showFullPath )
          folderPath = folderPath.mid( folderPath.findRev( '/' ) + 1 );

        KURLLabel *urlLabel = new KURLLabel( QString::null, folderPath,
                                             this );
        urlLabel->setAlignment( AlignLeft );
        urlLabel->show();
        // ### FIXME emit dcop signal to jumo to actual folder
        connect( urlLabel, SIGNAL( leftClickedURL() ), SLOT( raisePart() ) );
        mLayout->addWidget( urlLabel, counter, 0 );
        mLabels.append( urlLabel );
        QLabel *label = new QLabel( QString::number( numUnreadMsg ), this );

        label->setAlignment( AlignLeft );
        label->show();
        mLayout->addWidget( label, counter, 2 );
        mLabels.append( label );
        counter++;
      }
    }
    else {
      kdDebug(5602) << "Calling kmail->KMailIface->getFolder() via DCOP "
                       "failed." << endl;
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
