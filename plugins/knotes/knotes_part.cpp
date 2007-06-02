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

#include <qpopupmenu.h>
#include <qclipboard.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>

#include <libkdepim/infoextension.h>
#include <libkdepim/sidebarextension.h>

#include "knotes/knoteprinter.h"
#include "knotes/resourcemanager.h"

#include "knotes_part.h"
#include "knotes_part_p.h"
#include "knotetip.h"


KNotesPart::KNotesPart( QObject *parent, const char *name )
  : DCOPObject( "KNotesIface" ), KParts::ReadOnlyPart( parent, name ),
    mNotesView( new KNotesIconView() ),
    mNoteTip( new KNoteTip( mNotesView ) ),
    mNoteEditDlg( 0 ),
    mManager( new KNotesResourceManager() )
{
  mNoteList.setAutoDelete( true );

  setInstance( new KInstance( "knotes" ) );

  // create the actions
  new KAction( i18n( "&New" ), "knotes", CTRL+Key_N, this, SLOT( newNote() ),
               actionCollection(), "file_new" );
  new KAction( i18n( "Rename..." ), "text", this, SLOT( renameNote() ),
               actionCollection(), "edit_rename" );
  new KAction( i18n( "Delete" ), "editdelete", Key_Delete, this, SLOT( killSelectedNotes() ),
               actionCollection(), "edit_delete" );
  new KAction( i18n( "Print Selected Notes..." ), "print", CTRL+Key_P, this, SLOT( printSelectedNotes() ),
               actionCollection(), "print_note" );

  // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

  // set the view up
  mNotesView->setSelectionMode( QIconView::Extended );
  mNotesView->setItemsMovable( false );
  mNotesView->setResizeMode( QIconView::Adjust );
  mNotesView->setAutoArrange( true );
  mNotesView->setSorting( true );

  connect( mNotesView, SIGNAL( executed( QIconViewItem* ) ),
           this, SLOT( editNote( QIconViewItem* ) ) );
  connect( mNotesView, SIGNAL( returnPressed( QIconViewItem* ) ),
           this, SLOT( editNote( QIconViewItem* ) ) );
  connect( mNotesView, SIGNAL( itemRenamed( QIconViewItem* ) ),
           this, SLOT( renamedNote( QIconViewItem* ) ) );
  connect( mNotesView, SIGNAL( contextMenuRequested( QIconViewItem*, const QPoint& ) ),
           this, SLOT( popupRMB( QIconViewItem*, const QPoint& ) ) );
  connect( mNotesView, SIGNAL( onItem( QIconViewItem* ) ),
           this, SLOT( slotOnItem( QIconViewItem* ) ) );
  connect( mNotesView, SIGNAL( onViewport() ),
           this, SLOT( slotOnViewport() ) );
  connect( mNotesView, SIGNAL( currentChanged( QIconViewItem* ) ),
           this, SLOT( slotOnCurrentChanged( QIconViewItem* ) ) );

  slotOnCurrentChanged( 0 );

  new KParts::SideBarExtension( mNotesView, this, "NotesSideBarExtension" );

  setWidget( mNotesView );
  setXMLFile( "knotes_part.rc" );

  // connect the resource manager
  connect( mManager, SIGNAL( sigRegisteredNote( KCal::Journal* ) ),
           this, SLOT( createNote( KCal::Journal* ) ) );
  connect( mManager, SIGNAL( sigDeregisteredNote( KCal::Journal* ) ),
           this, SLOT( killNote( KCal::Journal* ) ) );

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
  QValueList<KCal::Journal*> journals;

  for ( QIconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      journals.append( static_cast<KNotesIconViewItem *>( it )->journal() );
    }
  }

  if ( journals.isEmpty() ) {
    KMessageBox::information( mNotesView, i18n("To print notes, first select the notes to print from the list."), i18n("Print Notes") );
    return;
  }

  KNotePrinter printer;
  printer.printNotes(journals );

#if 0
    QString content;
    if ( m_editor->textFormat() == PlainText )
        content = QStyleSheet::convertFromPlainText( m_editor->text() );
    else
        content = m_editor->text();

    KNotePrinter printer;
    printer.setMimeSourceFactory( m_editor->mimeSourceFactory() );
    //printer.setFont( m_config->font() );
    //printer.setContext( m_editor->context() );
    //printer.setStyleSheet( m_editor->styleSheet() );
    printer.setColorGroup( colorGroup() );
    printer.printNote( , content );
#endif
}

bool KNotesPart::openFile()
{
  return false;
}


// public KNotes DCOP interface implementation

QString KNotesPart::newNote( const QString& name, const QString& text )
{
  // create the new note
  KCal::Journal *journal = new KCal::Journal();

  // new notes have the current date/time as title if none was given
  if ( !name.isEmpty() )
      journal->setSummary( name );
  else
      journal->setSummary( KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );

  // the body of the note
  journal->setDescription( text );



  // Edit the new note if text is empty
  if ( text.isNull() )
  {
    if ( !mNoteEditDlg )
      mNoteEditDlg = new KNoteEditDlg( widget() );

    mNoteEditDlg->setTitle( journal->summary() );
    mNoteEditDlg->setText( journal->description() );

    if ( mNoteEditDlg->exec() == QDialog::Accepted )
    {
      journal->setSummary( mNoteEditDlg->title() );
      journal->setDescription( mNoteEditDlg->text() );
    }
    else
    {
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

QString KNotesPart::newNoteFromClipboard( const QString& name )
{
  const QString& text = KApplication::clipboard()->text();
  return newNote( name, text );
}

void KNotesPart::killNote( const QString& id )
{
  killNote( id, false );
}

void KNotesPart::killNote( const QString& id, bool force )
{
  KNotesIconViewItem *note = mNoteList[ id ];

  if ( note &&
       ( (!force && KMessageBox::warningContinueCancelList( mNotesView,
                    i18n( "Do you really want to delete this note?" ),
                    mNoteList[ id ]->text(), i18n( "Confirm Delete" ),
                    KStdGuiItem::del() ) == KMessageBox::Continue)
         || force )
     )
  {
    mManager->deleteNote( mNoteList[id]->journal() );
    mManager->save();
  }
}

QString KNotesPart::name( const QString& id ) const
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note )
    return note->text();
  else
    return QString::null;
}

QString KNotesPart::text( const QString& id ) const
{
  KNotesIconViewItem *note = mNoteList[id];
  if ( note )
    return note->journal()->description();
  else
    return QString::null;
}

void KNotesPart::setName( const QString& id, const QString& newName )
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    note->setText( newName );
    mManager->save();
  }
}

void KNotesPart::setText( const QString& id, const QString& newText )
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
  QDictIterator<KNotesIconViewItem> it( mNoteList );

  for ( ; it.current(); ++it )
    notes.insert( (*it)->journal()->uid(), (*it)->journal()->summary() );

  return notes;
}


// private stuff

void KNotesPart::killSelectedNotes()
{
  QPtrList<KNotesIconViewItem> items;
  QStringList notes;

  KNotesIconViewItem *knivi;
  for ( QIconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      knivi = static_cast<KNotesIconViewItem *>( it );
      items.append( knivi );
      notes.append( knivi->text() );
    }
  }

  if ( items.isEmpty() )
    return;

  int ret = KMessageBox::warningContinueCancelList( mNotesView,
            i18n( "Do you really want to delete this note?",
                  "Do you really want to delete these %n notes?", items.count() ),
            notes, i18n( "Confirm Delete" ),
            KStdGuiItem::del() );

  if ( ret == KMessageBox::Continue ) {
    QPtrListIterator<KNotesIconViewItem> kniviIt( items );
    while ( (knivi = *kniviIt) ) {
      ++kniviIt;
      mManager->deleteNote( knivi->journal() );
    }

    mManager->save();
  }
}

void KNotesPart::popupRMB( QIconViewItem *item, const QPoint& pos )
{
  QPopupMenu *contextMenu = NULL;

  if ( item )
    contextMenu = static_cast<QPopupMenu *>( factory()->container( "note_context", this ) );
  else
    contextMenu = static_cast<QPopupMenu *>( factory()->container( "notepart_context", this ) );

  if ( !contextMenu )
    return;

  contextMenu->popup( pos );
}

void KNotesPart::slotOnItem( QIconViewItem *i )
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
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "BgColor", "#ffff00" );

  property = journal->customProperty( "KNotes", "FgColor" );
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "FgColor", "#000000" );

  property = journal->customProperty( "KNotes", "RichText" );
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "RichText", "true" );

  mNoteList.insert( journal->uid(), new KNotesIconViewItem( mNotesView, journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
  mNoteList.remove( journal->uid() );
}

void KNotesPart::editNote( QIconViewItem *item )
{
  if ( !mNoteEditDlg )
    mNoteEditDlg = new KNoteEditDlg( widget() );

  KCal::Journal *journal = static_cast<KNotesIconViewItem *>( item )->journal();

  mNoteEditDlg->setRichText( journal->customProperty( "KNotes", "RichText" ) == "true" );
  mNoteEditDlg->setTitle( journal->summary() );
  mNoteEditDlg->setText( journal->description() );

  if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
    item->setText( mNoteEditDlg->title() );
    journal->setDescription( mNoteEditDlg->text() );
    mManager->save();
  }
}

void KNotesPart::renameNote()
{
  mNotesView->currentItem()->rename();
}

void KNotesPart::renamedNote( QIconViewItem* )
{
  mManager->save();
}

void KNotesPart::slotOnCurrentChanged( QIconViewItem* )
{
  KAction *renameAction = actionCollection()->action( "edit_rename" );
  KAction *deleteAction = actionCollection()->action( "edit_delete" );

  if ( !mNotesView->currentItem() ) {
    renameAction->setEnabled( false );
    deleteAction->setEnabled( false );
  } else {
    renameAction->setEnabled( true );
    deleteAction->setEnabled( true );
  }
}

#include "knotes_part.moc"
#include "knotes_part_p.moc"

