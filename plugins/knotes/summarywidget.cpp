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
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );
  QHBoxLayout *hbox = new QHBoxLayout( mainLayout, 3 );
  mLayout = new QGridLayout( mainLayout, 5, 2 );
  mainLayout->addStretch();

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( this );
  label->setFixedSize( 32, 32 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Desktop, KIcon::SizeMedium ) );
  hbox->addWidget( label );

  label = new QLabel( i18n( "Notes" ), this );
  label->setAlignment( AlignLeft );
  label->setFont( boldFont );
  hbox->addWidget( label );

  QString error;
  QCString appID;

  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "knotes" ) ) {
    if ( KApplication::startServiceByDesktopName( "knotes", QStringList(), &error, &appID ) != 0 ) {
      kdDebug() << error << endl;
      serviceAvailable = false;
    }
  }

  if ( serviceAvailable ) {
    mNotesMap = fetchNotes();
    updateView();
  }
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  DCOPRef dcopCall( "knotes", "KNotesIface" );

  int counter = 0;
  NotesMap::Iterator it;
  for ( it = mNotesMap.begin(); it != mNotesMap.end() && counter < 5; ++it ) {
    QString text;
    dcopCall.call( "text(QString)", it.key() ).get( text );
    if ( !text.isEmpty() ) {
      QLabel *label = new QLabel( this );
      label->setText( QString( "- %1:" ).arg( it.data() ) );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      KURLLabel *urlLabel = new KURLLabel( it.key(), text.left(
                                           text.find( "\n" ) ), this );
      mLayout->addWidget( urlLabel, counter, 1 );
      mLabels.append( urlLabel );
      
      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( urlClicked( const QString& ) ) );

      counter++;
    }
  }
}

void SummaryWidget::urlClicked( const QString &uid )
{
  DCOPRef dcopCall( "knotes", "KNotesIface" );
  dcopCall.send( "showNote(QString)", uid );
}

NotesMap SummaryWidget::fetchNotes()
{
  QCString replyType;
  QByteArray data, replyData;
  QDataStream arg(  data, IO_WriteOnly );
  if( kapp->dcopClient()->call( "knotes", "KNotesIface", "notes()", data, replyType, replyData ) ) {
    QDataStream answer(  replyData, IO_ReadOnly );
    NotesMap notes;
    answer >> notes;
    return notes;
  } else
    return NotesMap();
}

#include "summarywidget.moc"
