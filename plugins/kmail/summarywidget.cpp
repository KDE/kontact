/*
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

#include <dcopclient.h>
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
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kmail", KIcon::Desktop, KIcon::SizeMedium);
  QWidget *header = createHeader(this, icon, i18n("New Messages"));
  mLayout = new QGridLayout( 1, 3, 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);
  mainLayout->addStretch();

  connect( &mTimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
  mTimer.start( 0 );
}


void SummaryWidget::raisePart()
{

  // FIXME: select specific folder when 'selectFolder' dcop call is implemented
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
}

void SummaryWidget::timeout()
{
  mTimer.stop();

  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KConfig config( "kmailrc" );

  QStringList groups = config.groupList();
  groups.sort();

  int counter = 0;
  QStringList::Iterator it;
  for ( it = groups.begin(); it != groups.end() && counter < 6; ++it ) {
    if ( (*it).startsWith( QString::fromLatin1( "Folder-" ) ) ) { // hmm, that may fail...
      config.setGroup( *it );
      int numMsg = config.readNumEntry( "TotalMsgs", 0 );
      int numUnreadMsg = config.readNumEntry( "UnreadMsgs", 0 );
      if ( numUnreadMsg != 0 ) {
        QString folderPath = (*it).mid( 7 ).remove(0,1).replace(".directory","");
        KURLLabel *urlLabel = new KURLLabel( QString::null, folderPath, this );
        urlLabel->setAlignment( AlignLeft );
        urlLabel->show();
        // ### FIXME emit dcop signal to jumo to actual folder
        connect( urlLabel, SIGNAL( leftClickedURL() ), SLOT( raisePart() ) );
        mLayout->addWidget( urlLabel, counter, 0 );
        mLabels.append( urlLabel );
        QLabel *label = new QLabel( QString( "%1 / %2" ).arg( numUnreadMsg ).arg( numMsg ), this );

        label->setAlignment( AlignLeft );
        label->show();
        mLayout->addWidget( label, counter, 2 );
        mLabels.append( label );

        counter++;
      }
    }
  }

  if ( counter == 0 ) {
    QLabel *label = new QLabel( i18n( "No unread messages" ), this );
    label->show();
    mLayout->addMultiCellWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );
  }

  mTimer.start( 15 * 1000 );
}

#include "summarywidget.moc"
