/*
   This file is part of the KDE project
   Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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

#include <qlayout.h>
#include <qpopupmenu.h>
#include <qtextedit.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>

#include <libkdepim/infoextension.h>
#include <libkdepim/sidebarextension.h>

#include "knotes_part.h"

class NotesItem : public KListViewItem
{
  public:
    NotesItem( KListView *parent, KCal::Journal *journal )
      : KListViewItem( parent, "" ), mJournal( journal )
    {
      setRenameEnabled( 0, true );
      setPixmap( 0, KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Small ) );
    }

    KCal::Journal* journal() { return mJournal; }

    virtual void setText( int column, const QString &text )
    {
      if ( column == 0 )
        mJournal->setSummary( text );
    }

    virtual QString text( int column ) const
    {
      if ( column == 0 )
        return mJournal->summary();
      else if ( column == 1 )
        return mJournal->description().replace( "\n", " " );
      else
        return QString();
    }

  private:
    KCal::Journal* mJournal;
};

class NoteEditDialog : public KDialogBase
{
  public:
    NoteEditDialog( QWidget *parent, const QString &text )
      : KDialogBase( Plain, i18n( "Edit Note" ), Ok | Cancel, Ok,
                     parent, 0, true, true )
    {
      QWidget *page = plainPage();
      QVBoxLayout *layout = new QVBoxLayout( page );

      mTextEdit = new QTextEdit( page );
      layout->addWidget( mTextEdit );

      mTextEdit->setText( text );
      mTextEdit->setFocus();
    }

    QString text() const { return mTextEdit->text(); }

  private:
    QTextEdit *mTextEdit;
};

KNotesPart::KNotesPart( QObject *parent, const char *name )
  : KParts::ReadOnlyPart( parent, name ),
    mTicket( 0 ), mPopupMenu( 0 )
{
  setInstance( new KInstance( "knotes" ) );

  mCalendar = new KCal::CalendarResources;
  mResource = new KCal::ResourceLocal( ::locate( "data", "knotes/notes.ics" ) );
  mCalendar->resourceManager()->add( mResource );
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( slotCalendarChanged() ) );

  mNotesView = new KListView();
  mNotesView->setSelectionMode( QListView::Extended );
  mNotesView->addColumn( i18n( "Title" ) );
  mNotesView->addColumn( i18n( "Content" ) );
  mNotesView->setAllColumnsShowFocus( true );
  mNotesView->setResizeMode( QListView::LastColumn );

  (void) new KParts::SideBarExtension( mNotesView, this, "NotesSideBarExtension" );

  KStdAction::openNew( this, SLOT( newNote() ), actionCollection() );
  mActionEdit = new KAction( i18n( "Rename" ), "editrename", this,
                             SLOT( renameNote() ), actionCollection(),
                                     "edit_rename" );
  mActionDelete = new KAction( i18n( "Delete" ), "editdelete", 0, this,
                               SLOT( removeSelectedNotes() ), actionCollection(),
                               "edit_delete" );
  (void) new KAction( i18n( "Reload" ), "reload", 0, this,
                      SLOT( reloadNotes() ), actionCollection(), "view_refresh" );

  connect( mNotesView, SIGNAL( doubleClicked( QListViewItem*, const QPoint&, int ) ),
           this, SLOT( editNote( QListViewItem*, const QPoint&, int ) ) );
  connect( mNotesView, SIGNAL( returnPressed( QListViewItem* ) ),
           this, SLOT( editNote( QListViewItem* ) ) );
  connect( mNotesView, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
           this, SLOT( popupRMB( QListViewItem*, const QPoint&, int ) ) );
  connect( mNotesView, SIGNAL( itemRenamed( QListViewItem*, int, const QString& ) ),
           this, SLOT( noteRenamed( QListViewItem*, int, const QString& ) ) );

  setWidget( mNotesView );

  mAppIcon = KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Small );

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "KNoteInfoExtension" );
  connect( this, SIGNAL( noteSelected( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );
  connect( this, SIGNAL( noteSelected( const QPixmap& ) ),
           info, SIGNAL( iconChanged( const QPixmap& ) ) );

  setXMLFile( "knotes_part.rc" );

  reloadNotes();
}

KNotesPart::~KNotesPart()
{
}

void KNotesPart::reloadNotes()
{
  int pos = mNotesView->itemPos( mNotesView->currentItem() );
  mNotesView->clear();

  KCal::Journal::List::Iterator it;
  KCal::Journal::List notes = mCalendar->journals();
  for ( it = notes.begin(); it != notes.end(); ++it )
    (void) new NotesItem( mNotesView, (*it) );

  mNotesView->setCurrentItem( mNotesView->itemAt( QPoint( 1, pos ) ) );
}

bool KNotesPart::openFile()
{
  return false;
}

void KNotesPart::popupRMB( QListViewItem *item, const QPoint& pos, int )
{
  mPopupMenu = static_cast<QPopupMenu*>( factory()->container( "notePopup", this ) );
  if ( !mPopupMenu )
    return;

  bool state = ( item != 0 );
  mActionEdit->setEnabled( state );
  mActionDelete->setEnabled( state );

  mPopupMenu->popup( pos );
}

void KNotesPart::removeNote()
{
  NotesItem *item = static_cast<NotesItem*>( mNotesView->currentItem() );

  if ( !item )
    return;

  if ( !lock() )
    return;

  mCalendar->deleteJournal( item->journal() );

  unlock();
}

void KNotesPart::removeSelectedNotes()
{
  QListViewItemIterator it( mNotesView );
  QPtrList<NotesItem> items;
  QStringList titles;

  while ( it.current() ) {
    if ( it.current()->isSelected() ) {
      NotesItem *item = static_cast<NotesItem*>( it.current() );
      items.append( item );
      titles.append( item->journal()->summary() );
    }

    ++it;
  }

  if ( items.isEmpty() )
    return;

  if ( !lock() )
    return;

  int ret = KMessageBox::warningContinueCancelList( mNotesView,
      i18n( "Do you really want to delete that note?",
            "Do you really want to delete these %n notes?", items.count() ),
      titles,
      i18n( "Confirm Delete" ),
      i18n( "Delete" )
      );

  if ( ret == KMessageBox::Continue ) {
    QPtrListIterator<NotesItem> itemIt( items );
    NotesItem *item;
    while ( (item = itemIt.current()) != 0 ) {
      ++itemIt;

      mCalendar->deleteJournal( item->journal() );

      delete item;
    }
  }

  unlock();
}

void KNotesPart::renameNote()
{
  if ( mNotesView->currentItem() )
    mNotesView->currentItem()->startRename( 0 );
}

void KNotesPart::noteRenamed( QListViewItem *i, int,  const QString& )
{
  NotesItem *item = static_cast<NotesItem*>( i );

  if ( !item )
    return;

  if ( !lock() )
    return;

  unlock();
}

void KNotesPart::editNote( QListViewItem *i, const QPoint&, int column )
{
  if ( column == 1 )
    editNote( i );
}

void KNotesPart::editNote( QListViewItem *i )
{
  NotesItem *item = static_cast<NotesItem*>( i );

  if ( !item )
    return;

  if ( !lock() )
    return;

  NoteEditDialog dlg( mNotesView, item->journal()->description() );
  if ( dlg.exec() ) {
    item->journal()->setDescription( dlg.text() );
  }

  unlock();
}

void KNotesPart::newNote()
{
  bool ok;
  QString title = KInputDialog::getText( i18n( "Title" ),
                                         i18n( "Title" ),
                                         KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ),
                                         &ok );
  if ( !ok )
    return;

  if ( !lock() )
    return;

  NoteEditDialog dlg( mNotesView, "" );
  if ( dlg.exec() ) {
    KCal::Journal* journal = new KCal::Journal();
    mCalendar->addJournal( journal );
    journal->setSummary( title );
    journal->setDescription( dlg.text() );
  }

  unlock();
}

void KNotesPart::slotCalendarChanged()
{
  reloadNotes();
}

bool KNotesPart::lock()
{
  if ( mTicket ) // we still have a valid ticket
    return true;

  mTicket = mCalendar->requestSaveTicket( mResource );

  bool ok = (mTicket != 0);

  if ( !ok )
    KMessageBox::error( mNotesView,
                        i18n( "Unable to access the notes, make sure no other program uses them." ) );

  return ok;
}

bool KNotesPart::unlock()
{
  if ( !mTicket ) {
    kdError() << "save with null ticket" << endl;
    return false;
  }

  mCalendar->save( mTicket );
  mTicket = 0;

  return true;
}

#include "knotes_part.moc"
