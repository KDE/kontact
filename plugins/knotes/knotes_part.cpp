/*
  This file is part of the KDE project

  Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (C) 2004-2006 Michael Brade <brade@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "knotes_part.h"
#include "knotes_part_p.h"
#include "knotetip.h"
#include "knotesadaptor.h"

#include <kactioncollection.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kicon.h>
#include "knotes/knoteprinter.h"
#include "knotes/resourcemanager.h"

#include <QMenu>
#include <QClipboard>
#include <QApplication>

KNotesPart::KNotesPart( QObject *parent )
  :  KParts::ReadOnlyPart( parent ), mNotesView( new KNotesIconView() ),
     mNoteTip( new KNoteTip( mNotesView ) ), mNoteEditDlg( 0 ),
     mManager( new KNotesResourceManager() )
{
  (void) new KNotesAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/KNotes", this );
  mNoteList.setAutoDelete( true );

  setComponentData( KComponentData( "knotes" ) );

  // create the actions
  KAction *action = new KAction( KIcon( "knotes" ), i18n( "&New" ), this );
  actionCollection()->addAction( "file_new", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(newNote()) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );

  action = new KAction( KIcon( "document-edit" ), i18n( "Edit..." ), this );
  actionCollection()->addAction( "edit_note", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(editNote()) );

  action = new KAction( KIcon( "edit-rename" ), i18n( "Rename..." ), this );
  actionCollection()->addAction( "edit_rename", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(renameNote()) );

  action = new KAction( KIcon( "edit-delete" ), i18n( "Delete" ), this );
  actionCollection()->addAction( "edit_delete", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(killSelectedNotes()) );
  action->setShortcut( QKeySequence( Qt::Key_Delete ) );

  action = new KAction( KIcon( "document-print" ), i18n( "Print Selected Notes..." ), this );
  actionCollection()->addAction( "print_note", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(printSelectedNotes()) );

  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Delete ) );

  // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

  // set the view up
  mNotesView->setSelectionMode( Q3IconView::Extended );
  mNotesView->setItemsMovable( false );
  mNotesView->setResizeMode( Q3IconView::Adjust );
  mNotesView->setSorting( true );

  connect( mNotesView, SIGNAL(executed(Q3IconViewItem*)),
           this, SLOT(editNote(Q3IconViewItem*)) );
  connect( mNotesView, SIGNAL(returnPressed(Q3IconViewItem*)),
           this, SLOT(editNote(Q3IconViewItem*)) );
  connect( mNotesView, SIGNAL(itemRenamed(Q3IconViewItem*)),
           this, SLOT(renamedNote(Q3IconViewItem*)) );
  connect( mNotesView, SIGNAL(contextMenuRequested(Q3IconViewItem*,const QPoint&)),
           this, SLOT(popupRMB(Q3IconViewItem*,const QPoint&)) );
  connect( mNotesView, SIGNAL(onItem(Q3IconViewItem*)),
           this, SLOT(slotOnItem(Q3IconViewItem*)) );
  connect( mNotesView, SIGNAL(onViewport()),
           this, SLOT(slotOnViewport()) );
  connect( mNotesView, SIGNAL(currentChanged(Q3IconViewItem*)),
           this, SLOT(slotOnCurrentChanged(Q3IconViewItem*)) );

  slotOnCurrentChanged( 0 );

  setWidget( mNotesView );
  setXMLFile( "knotes_part.rc" );

  // connect the resource manager
  connect( mManager, SIGNAL(sigRegisteredNote(KCal::Journal*)),
           this, SLOT(createNote(KCal::Journal*)) );
  connect( mManager, SIGNAL(sigDeregisteredNote(KCal::Journal*)),
           this, SLOT(killNote(KCal::Journal*)) );

  // read the notes
  mManager->load();
}

KNotesPart::~KNotesPart()
{
  delete mNoteTip;
  mNoteTip = 0;

  delete mManager;
  mManager = 0;
}

void KNotesPart::printSelectedNotes()
{
  QList<KCal::Journal*> journals;

  for ( Q3IconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      journals.append( static_cast<KNotesIconViewItem *>( it )->journal() );
    }
  }

  if ( journals.isEmpty() ) {
    KMessageBox::information(
      mNotesView,
      i18n( "To print notes, first select the notes to print from the list." ),
      i18n( "Print Popup Notes" ) );
    return;
  }

  KNotePrinter printer;
  printer.printNotes( journals );

}

bool KNotesPart::openFile()
{
  return false;
}

// public KNotes D-Bus interface implementation

QString KNotesPart::newNote( const QString &name, const QString &text )
{
  // create the new note
  KCal::Journal *journal = new KCal::Journal();

  // new notes have the current date/time as title if none was given
  if ( !name.isEmpty() ) {
    journal->setSummary( name );
  } else {
    journal->setSummary( KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
  }

  // the body of the note
  journal->setDescription( text );

  // Edit the new note if text is empty
  if ( text.isNull() ) {
    if ( !mNoteEditDlg ) {
      mNoteEditDlg = new KNoteEditDlg( widget() );
    }

    mNoteEditDlg->setTitle( journal->summary() );
    mNoteEditDlg->setText( journal->description() );

    if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
      journal->setSummary( mNoteEditDlg->title() );
      journal->setDescription( mNoteEditDlg->text() );
    } else {
      delete journal;
      return "";
    }
  }

  mManager->addNewNote( journal );
  mManager->save();

  KNotesIconViewItem *note = mNoteList[ journal->uid() ];
  mNotesView->ensureItemVisible( note );
  mNotesView->setCurrentItem( note );

  return journal->uid();
}

QString KNotesPart::newNoteFromClipboard( const QString &name )
{
  const QString &text = QApplication::clipboard()->text();
  return newNote( name, text );
}

void KNotesPart::killNote( const QString &id )
{
  killNote( id, false );
}

void KNotesPart::killNote( const QString &id, bool force )
{
  KNotesIconViewItem *note = mNoteList[ id ];

  if ( note &&
       ( (!force && KMessageBox::warningContinueCancelList(
            mNotesView,
            i18n( "Do you really want to delete this note?" ),
            QStringList( mNoteList[ id ]->text() ), i18n( "Confirm Delete" ),
            KStandardGuiItem::del() ) == KMessageBox::Continue )
         || force ) ) {
    mManager->deleteNote( mNoteList[id]->journal() );
    mManager->save();
  }
}

QString KNotesPart::name( const QString &id ) const
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    return note->text();
  } else {
    return QString();
  }
}

QString KNotesPart::text( const QString &id ) const
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    return note->journal()->description();
  } else {
    return QString();
  }
}

void KNotesPart::setName( const QString &id, const QString &newName )
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    note->setText( newName );
    mManager->save();
  }
}

void KNotesPart::setText( const QString &id, const QString &newText )
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    note->journal()->setDescription( newText );
    mManager->save();
  }
}

QMap<QString, QString> KNotesPart::notes() const
{
  QMap<QString, QString> notes;
  Q3DictIterator<KNotesIconViewItem> it( mNoteList );

  for ( ; it.current(); ++it ) {
    notes.insert( (*it)->journal()->uid(), (*it)->journal()->summary() );
  }

  return notes;
}

// private stuff

void KNotesPart::killSelectedNotes()
{
  QList<KNotesIconViewItem*> items;
  QStringList notes;

  KNotesIconViewItem *knivi;
  for ( Q3IconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      knivi = static_cast<KNotesIconViewItem *>( it );
      items.append( knivi );
      notes.append( knivi->text() );
    }
  }

  if ( items.isEmpty() ) {
    return;
  }

  int ret = KMessageBox::warningContinueCancelList(
    mNotesView,
    i18np( "Do you really want to delete this note?",
           "Do you really want to delete these %1 notes?", items.count() ),
    notes, i18n( "Confirm Delete" ),
    KStandardGuiItem::del() );

  if ( ret == KMessageBox::Continue ) {
    QListIterator<KNotesIconViewItem*> kniviIt( items );
    while ( kniviIt.hasNext() ) {
      mManager->deleteNote( kniviIt.next()->journal() );
    }

    mManager->save();
  }
}

void KNotesPart::popupRMB( Q3IconViewItem *item, const QPoint &pos )
{
  QMenu *contextMenu = 0;

  if ( item ) {
    contextMenu = static_cast<QMenu *>( factory()->container( "note_context", this ) );
  } else {
    contextMenu = static_cast<QMenu *>( factory()->container( "notepart_context", this ) );
  }

  if ( !contextMenu ) {
    return;
  }

  contextMenu->popup( pos );
}

void KNotesPart::slotOnItem( Q3IconViewItem *i )
{
  // TODO: disable (i.e. setNote( QString::null )) when mouse button pressed

  KNotesIconViewItem *item = static_cast<KNotesIconViewItem *>( i );
  mNoteTip->setNote( item );
}

void KNotesPart::slotOnViewport()
{
  mNoteTip->setNote( 0 );
}

// TODO: also with takeItem, clear(),

// create and kill the icon view item corresponding to the note, edit the note

void KNotesPart::createNote( KCal::Journal *journal )
{
  // make sure all fields are existent, initialize them with default values
  QString property = journal->customProperty( "KNotes", "BgColor" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "BgColor", "#ffff00" );
  }

  property = journal->customProperty( "KNotes", "FgColor" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "FgColor", "#000000" );
  }

  property = journal->customProperty( "KNotes", "RichText" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "RichText", "true" );
  }

  mNoteList.insert( journal->uid(), new KNotesIconViewItem( mNotesView, journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
  mNoteList.remove( journal->uid() );
}

void KNotesPart::editNote( Q3IconViewItem *item )
{
  if ( !mNoteEditDlg ) {
    mNoteEditDlg = new KNoteEditDlg( widget() );
  }

  KCal::Journal *journal = static_cast<KNotesIconViewItem *>( item )->journal();
  mNoteEditDlg->setTitle( journal->summary() );
  mNoteEditDlg->setText( journal->description() );

  if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
    item->setText( mNoteEditDlg->title() );
    journal->setDescription( mNoteEditDlg->text() );
    mManager->save();
  }
}

void KNotesPart::editNote()
{
  if ( mNotesView->currentItem() )
  {
    editNote( mNotesView->currentItem() );
  }
}

void KNotesPart::renameNote()
{
  mNotesView->currentItem()->rename();
}

void KNotesPart::renamedNote( Q3IconViewItem * )
{
  mManager->save();
}

void KNotesPart::slotOnCurrentChanged( Q3IconViewItem * )
{
  QAction *renameAction = actionCollection()->action( "edit_rename" );
  QAction *deleteAction = actionCollection()->action( "edit_delete" );
  QAction *editAction = actionCollection()->action( "edit_note" );
  if ( !mNotesView->currentItem() ) {
    renameAction->setEnabled( false );
    deleteAction->setEnabled( false );
    editAction->setEnabled( false );
  } else {
    renameAction->setEnabled( true );
    deleteAction->setEnabled( true );
    editAction->setEnabled( true );
  }
}

#include "knotes_part.moc"
#include "knotes_part_p.moc"

