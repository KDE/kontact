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

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name )
{
  setPaletteBackgroundColor( QColor( 240, 240, 240 ) );

  QGridLayout *layout = new QGridLayout( this, 3, 2, 3 );
  layout->setRowStretch( 2, 1 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Desktop, KIcon::SizeMedium ) );
  label->setAlignment( AlignLeft | AlignTop );
  layout->addWidget( label, 0, 0 );

  label = new QLabel( i18n( "Notes" ), this );
  label->setAlignment( AlignRight | AlignTop );
  label->setFont( boldFont );
  layout->addWidget( label, 0, 1 );

  mNoteList = new QLabel( this );
  mNoteList->setAlignment( AlignLeft | AlignTop );
  layout->addMultiCellWidget( mNoteList, 1, 1, 0, 1 );

  QString error;
  QCString appID;

  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "knotes" ) ) {
    if ( KApplication::startServiceByDesktopName( "knotes", QStringList(), &error, &appID ) != 0 ) {
      mNoteList->setText( error );
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
  DCOPRef dcopCall( "knotes", "KNotesIface" );

  QString lines;

  NotesMap::Iterator it;
  for ( it = mNotesMap.begin(); it != mNotesMap.end(); ++it ) {
    QString text;
    dcopCall.call( "text(QString)", it.key() ).get( text );
    if ( !text.isEmpty() ) {
      lines += QString( "<li><nobr><b>%1:</b> %2</nobr></li>" )
                      .arg( it.data() )
                      .arg( text.left( text.find( "\n" ) ) );
    }
  }

  mNoteList->setText( "<ul>" + lines + "</ul>" );
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
