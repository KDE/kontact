/*
   This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qtextedit.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <libkdepim/infoextension.h>

#include "knotes_part.h"

class NotesItem : public KListViewItem
{
  public:
    NotesItem( KListView *parent, const QString &id, const QString &text );
    QString id() { return noteID; };
  private:
    QString noteID;
};

NotesItem::NotesItem( KListView *parent, const QString &id, const QString &text )
  :	KListViewItem( parent, text )
{
  noteID = id;
  setRenameEnabled( 0, true );

  setPixmap( 0, KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Small ) );
}

KNotesPart::KNotesPart( QObject *parent, const char *name )
  : KParts::ReadOnlyPart( parent, name ),
    mPopupMenu( new QPopupMenu ),
    mNoteChanged( false )
{
  QSplitter *splitter = new QSplitter( Qt::Horizontal );

  mNotesView = new KListView( splitter );
  mNotesView->setSelectionMode( QListView::Extended );
  mNotesView->addColumn( i18n( "Title" ) );

  mNotesEdit = new QTextEdit( splitter );

  mPopupMenu->insertItem( BarIcon( "filenew" ), i18n( "New" ),
                          this, SLOT( newNote() ), 1, 1);
  mPopupMenu->insertItem( BarIcon( "editrename" ), i18n( "Rename" ),
                          this, SLOT( renameNote() ), 2, 2 );
  mPopupMenu->insertItem( BarIcon( "editdelete" ), i18n( "Remove" ),
                          this, SLOT( removeSelectedNotes() ), 3, 3 );
  mPopupMenu->insertSeparator();
  mPopupMenu->insertItem( BarIcon( "reload" ), i18n( "Reload" ),
                          this, SLOT( reloadNotes() ), 4, 4 );

  connect( mNotesView, SIGNAL( selectionChanged( QListViewItem* ) ),
           this, SLOT( showNote( QListViewItem* ) ) );
  connect( mNotesView, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
           this, SLOT( popupRMB( QListViewItem*, const QPoint&, int ) ) );
  connect( mNotesView, SIGNAL( itemRenamed( QListViewItem*, int, const QString& ) ),
           this, SLOT( noteRenamed( QListViewItem*, int, const QString& ) ) );
  connect( mNotesEdit, SIGNAL( textChanged() ),
           this, SLOT( noteChanged() ) );

  reloadNotes();
  setWidget( splitter );

  mAppIcon = KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Small );

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "KNoteInfoExtension" );
  connect( this, SIGNAL( noteSelected( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );
  connect( this, SIGNAL( noteSelected( const QPixmap& ) ),
           info, SIGNAL( iconChanged( const QPixmap& ) ) );
}

KNotesPart::~KNotesPart()
{
  saveNote();
}

void KNotesPart::reloadNotes()
{
  if ( !kapp->dcopClient()->isApplicationRegistered( "knotes" ) ) {
    QString *error = 0;
    int started  = KApplication::startServiceByDesktopName( "knotes",
                                                            QString(), error );

    if ( started > 0 ) {
      if ( error )
        KMessageBox::error( 0L, *error, i18n( "Error" ) );
      return;
    }

    delete error;
  }

  mNotesView->clear();

  NotesMap map;

  QCString replyType;
  QByteArray data, replyData;
  QDataStream arg( data, IO_WriteOnly );
  if ( kapp->dcopClient()->call( "knotes", "KNotesIface", "notes()", data, replyType, replyData ) ) {
    kdDebug(5602) << "Reply Type: " << replyType << endl;
    QDataStream answer( replyData, IO_ReadOnly );
    answer >> map;
  }

  NotesMap::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it )
    (void) new NotesItem( mNotesView, it.key(), it.data() );

  mNotesView->setCurrentItem( mNotesView->firstChild() );
  showNote( mNotesView->firstChild() );
}

bool KNotesPart::openFile()
{
  return false;
}

void KNotesPart::popupRMB( QListViewItem *item, const QPoint& pos, int )
{
  if ( item ) {
    mPopupMenu->setItemVisible( 2, true );
    mPopupMenu->setItemEnabled( 2, true );
    mPopupMenu->setItemVisible( 3, true );
    mPopupMenu->setItemEnabled( 3, true );
  } else {
    mPopupMenu->setItemVisible( 2, false );
    mPopupMenu->setItemEnabled( 2, false );
    mPopupMenu->setItemVisible( 3, false );
    mPopupMenu->setItemEnabled( 3, false );
  }

  mPopupMenu->popup( pos );
}

void KNotesPart::removeNote()
{
  NotesItem *item = static_cast<NotesItem*>( mNotesView->currentItem() );

  if ( !item )
    return;

  DCOPRef dcopCall( "knotes", "KNotesIface" );
  dcopCall.call( "killNote(QString, bool)", item->id(), true );

  reloadNotes();
}

void KNotesPart::removeSelectedNotes()
{
  QStringList ids;
  QStringList names;

  QListViewItemIterator it( mNotesView );
  while ( it.current() ) {
    if ( it.current()->isSelected() ) {
      ids += static_cast<NotesItem*>( it.current() )->id();
      names += it.current()->text( 0 );
    }

    ++it;
  }

  if ( ids.isEmpty() )
    return;

  if ( ids.count() == 1 ) {
    DCOPRef dcopCall( "knotes", "KNotesIface" );
    dcopCall.call( "killNote(QString)", ids.first() );
  } else {
    int ret = KMessageBox::warningContinueCancelList( 0,
        i18n( "translators: not called for n == 1", "Do you really want to delete these %n notes?", ids.count() ),
        names,
        i18n( "Confirm Delete" ),
        i18n( "Delete" ) );

    int doIt = ( ret == KMessageBox::Continue );

    if ( doIt )
      for ( QStringList::ConstIterator it = ids.begin(); it != ids.end(); ++it ) {
        DCOPRef dcopCall( "knotes", "KNotesIface" );
        dcopCall.call( "killNote(QString, bool)", *it, true );
      }
  }

  reloadNotes();
}

void KNotesPart::renameNote()
{
  if ( mNotesView->currentItem() )
    mNotesView->currentItem()->startRename( 0 );
}

void KNotesPart::noteRenamed( QListViewItem *i, int,  const QString& text )
{
  NotesItem *item = static_cast<NotesItem*>( i );

  if ( !item )
    return;

  DCOPRef dcopCall( "knotes", "KNotesIface" );
  dcopCall.send( "setName(QString,QString)", item->id(), text );
}

void KNotesPart::showNote( QListViewItem *i )
{
  if ( !mCurrentNote.isEmpty() ) {
    if ( mNoteChanged )
      saveNote();
  }

  mNotesEdit->clear();

  NotesItem *item = static_cast<NotesItem*>( i );
  if ( !item ) {
    mCurrentNote = "";
    return;
  }

  mCurrentNote = item->id();

  DCOPRef dcopCall( "knotes", "KNotesIface" );
  mNotesEdit->blockSignals( true );
  mNotesEdit->setText( dcopCall.call( "text(QString)", item->id() ) );
  mNotesEdit->blockSignals( false );

  emit noteSelected( item->text( 0 ) );
  emit noteSelected( mAppIcon );
}

void KNotesPart::noteChanged()
{
  mNoteChanged = true;  
}

void KNotesPart::saveNote()
{
  if ( mCurrentNote.isEmpty() )
    return;

  DCOPRef dcopCall( "knotes", "KNotesIface" );
  dcopCall.send( "setText(QString,QString)", mCurrentNote, mNotesEdit->text() );

  mNoteChanged = false;
}

void KNotesPart::newNote()
{
  DCOPRef dcopCall( "knotes", "KNotesIface" );
  dcopCall.call( "newNote(QString, QString)", QString::null, QString::null );

  reloadNotes();
}

#include "knotes_part.moc"
