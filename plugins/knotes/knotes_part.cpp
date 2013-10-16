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
#include "knotesadaptor.h"
#include "knotetip.h"
#include "knotes/knoteprinter.h"
#include "knotes/resourcemanager.h"

#include <KAction>
#include <KInputDialog>
#include <KMessageBox>

#include <QApplication>
#include <QClipboard>
#include <QMenu>

KNotesIconView::KNotesIconView( KNotesPart *part )
  : KListWidget(), m_part( part )
{
  setViewMode( QListView::IconMode );
  setMovement( QListView::Static );
  setSortingEnabled( true );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setWordWrap( true );
  setMouseTracking ( true );
}

void KNotesIconView::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::RightButton ) {
    QListWidget::mousePressEvent( e );
    m_part->popupRMB( currentItem(), e->pos (), e->globalPos() );
  } else {
    KListWidget::mousePressEvent( e );
  }
}

KNotesPart::KNotesPart( QObject *parent )
  :  KParts::ReadOnlyPart( parent ), mNotesView( new KNotesIconView(this) ),
     mNoteTip( new KNoteTip( mNotesView ) ), mNoteEditDlg( 0 ),
     mManager( new KNotesResourceManager() )
{
  (void) new KNotesAdaptor( this );
  QDBusConnection::sessionBus().registerObject( QLatin1String("/KNotes"), this );

  setComponentData( KComponentData( "knotes" ) );

  // create the actions
  KAction *action =
    new KAction( KIcon( QLatin1String("knotes") ),
                 i18nc( "@action:inmenu create new popup note", "&New" ), this );
  actionCollection()->addAction( QLatin1String("file_new"), action );
  connect( action, SIGNAL(triggered(bool)), SLOT(newNote()) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setHelpText(
    i18nc( "@info:status", "Create a new popup note" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can add a new popup note." ) );

  action = new KAction( KIcon( QLatin1String("document-edit") ),
                        i18nc( "@action:inmenu", "Edit..." ), this );
  actionCollection()->addAction( QLatin1String("edit_note"), action );
  connect( action, SIGNAL(triggered(bool)), SLOT(editNote()) );
  action->setHelpText(
    i18nc( "@info:status", "Edit popup note" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can modify an existing popup note." ) );

  action = new KAction( KIcon( QLatin1String("edit-rename") ),
                        i18nc( "@action:inmenu", "Rename..." ), this );
  actionCollection()->addAction( QLatin1String("edit_rename"), action );
  connect( action, SIGNAL(triggered(bool)), SLOT(renameNote()) );
  action->setHelpText(
    i18nc( "@info:status", "Rename popup note" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can rename an existing popup note." ) );

  action = new KAction( KIcon( QLatin1String("edit-delete") ),
                        i18nc( "@action:inmenu", "Delete" ), this );
  actionCollection()->addAction( QLatin1String("edit_delete"), action );
  connect( action, SIGNAL(triggered(bool)), SLOT(killSelectedNotes()) );
  action->setShortcut( QKeySequence( Qt::Key_Delete ) );
  action->setHelpText(
    i18nc( "@info:status", "Delete popup note" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be prompted if you really want to permanently remove "
           "the selected popup note." ) );

  action = new KAction( KIcon( QLatin1String("document-print") ),
                        i18nc( "@action:inmenu", "Print Selected Notes..." ), this );
  actionCollection()->addAction( QLatin1String("print_note"), action );
  connect( action, SIGNAL(triggered(bool)), SLOT(printSelectedNotes()) );
  action->setHelpText(
    i18nc( "@info:status", "Print popup note" ) );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be prompted to print the selected popup note." ) );

  // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

  // set the view up

  connect( mNotesView, SIGNAL(executed(QListWidgetItem*)),
           this, SLOT(editNote(QListWidgetItem*)) );

  connect( mNotesView, SIGNAL(entered(QModelIndex)),
            this, SLOT(requestToolTip(QModelIndex)));

  connect( mNotesView, SIGNAL(viewportEntered()),
            this, SLOT(hideToolTip()));

  connect( mNotesView, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
           this, SLOT(slotOnCurrentChanged()) );

  slotOnCurrentChanged();

  setWidget( mNotesView );
  setXMLFile( QLatin1String("knotes_part.rc") );

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

void KNotesPart::requestToolTip( const QModelIndex &index )
{
  QRect m_itemRect = mNotesView->visualRect( index );
  mNoteTip->setNote(
    static_cast<KNotesIconViewItem *>( mNotesView->itemAt( m_itemRect.topLeft() ) ) );
}

void KNotesPart::hideToolTip()
{
  mNoteTip->setNote( 0 );
}

void KNotesPart::printSelectedNotes()
{
  QList<Journal*> journals;
  QList<QListWidgetItem *> lst = mNotesView->selectedItems();
  if ( lst.isEmpty() ) {
    KMessageBox::information(
      mNotesView,
      i18nc( "@info",
             "To print notes, first select the notes to print from the list." ),
      i18nc( "@title:window", "Print Popup Notes" ) );
    return;
  }

  foreach ( QListWidgetItem *item, lst ) {
    journals.append( static_cast<KNotesIconViewItem *>( item )->journal() );
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
  Journal *journal = new Journal();

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
    delete mNoteEditDlg;
    mNoteEditDlg = new KNoteEditDlg( widget() );

    mNoteEditDlg->setTitle( journal->summary() );
    mNoteEditDlg->setText( journal->description() );

    mNoteEditDlg->noteEdit()->setFocus();
    if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
      journal->setSummary( mNoteEditDlg->title() );
      journal->setDescription( mNoteEditDlg->text() );
    } else {
      delete journal;
        return QString();
    }
  }

  mManager->addNewNote( journal );

  KNotesIconViewItem *note = mNoteList.value( journal->uid() );
  mNotesView->scrollToItem( note );
  mNotesView->setCurrentItem( note );
  mManager->save();
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
  KNotesIconViewItem *note = mNoteList.value( id );

  if ( note &&
       ( (!force && KMessageBox::warningContinueCancelList(
            mNotesView,
            i18nc( "@info", "Do you really want to delete this note?" ),
            QStringList( mNoteList.value( id )->text() ),
            i18nc( "@title:window", "Confirm Delete" ),
            KStandardGuiItem::del() ) == KMessageBox::Continue )
         || force ) ) {
    mManager->deleteNote( mNoteList.value( id )->journal() );
    mManager->save();
  }
}

QString KNotesPart::name( const QString &id ) const
{
  KNotesIconViewItem *note = mNoteList.value( id );
  if ( note ) {
    return note->text();
  } else {
    return QString();
  }
}

QString KNotesPart::text( const QString &id ) const
{
  KNotesIconViewItem *note = mNoteList.value( id );
  if ( note ) {
    return note->journal()->description();
  } else {
    return QString();
  }
}

void KNotesPart::setName( const QString &id, const QString &newName )
{
  KNotesIconViewItem *note = mNoteList.value( id );
  if ( note ) {
    note->setIconText( newName );
    mManager->save();
  }
}

void KNotesPart::setText( const QString &id, const QString &newText )
{
  KNotesIconViewItem *note = mNoteList.value( id );
  if ( note ) {
    note->journal()->setDescription( newText );
    mManager->save();
  }
}

QMap<QString, QString> KNotesPart::notes() const
{
  QMap<QString, QString> notes;

  QHashIterator<QString, KNotesIconViewItem*> i(mNoteList);
  while ( i.hasNext() ) {
    i.next();
    notes.insert( i.value()->journal()->uid(), i.value()->journal()->summary() );
  }
  return notes;
}

// private stuff

void KNotesPart::killSelectedNotes()
{
  QList<KNotesIconViewItem*> items;
  QStringList notes;
  QList<QListWidgetItem *> lst = mNotesView->selectedItems ();
  if ( lst.isEmpty() ) {
    return;
  }

  foreach ( QListWidgetItem *item, lst ) {
    KNotesIconViewItem *knivi = static_cast<KNotesIconViewItem *>( item );
    items.append( knivi );
    notes.append( knivi->realName() );
  }

  int ret = KMessageBox::warningContinueCancelList(
    mNotesView,
    i18ncp( "@info",
            "Do you really want to delete this note?",
            "Do you really want to delete these %1 notes?", items.count() ),
    notes, i18nc( "@title:window", "Confirm Delete" ),
    KStandardGuiItem::del() );

  if ( ret == KMessageBox::Continue ) {
    QListIterator<KNotesIconViewItem*> kniviIt( items );
    while ( kniviIt.hasNext() ) {
      mManager->deleteNote( kniviIt.next()->journal() );
    }

    mManager->save();
  }
}

void KNotesPart::popupRMB( QListWidgetItem *item, const QPoint &pos, const QPoint &globalPos )
{
  Q_UNUSED( item );

  QMenu *contextMenu = 0;
  if ( mNotesView->itemAt ( pos ) ) {
    contextMenu = static_cast<QMenu *>( factory()->container( QLatin1String("note_context"), this ) );
  } else {
    contextMenu = static_cast<QMenu *>( factory()->container( QLatin1String("notepart_context"), this ) );
  }

  if ( !contextMenu ) {
    return;
  }

  contextMenu->popup( mNotesView->mapFromParent( globalPos ) );
}

void KNotesPart::mouseMoveOnListWidget( const QPoint & pos )
{
  QListWidgetItem *item = mNotesView->itemAt( pos );
  mNoteTip->setNote( dynamic_cast<KNotesIconViewItem *>( item ) );
}

// TODO: also with takeItem, clear(),

// create and kill the icon view item corresponding to the note, edit the note

void KNotesPart::createNote( KCal::Journal *journal )
{
  // make sure all fields are existent, initialize them with default values
  QString property = journal->customProperty( "KNotes", "BgColor" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "BgColor", QLatin1String("#ffff00") );
  }

  property = journal->customProperty( "KNotes", "FgColor" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "FgColor", QLatin1String("#000000" ));
  }

  property = journal->customProperty( "KNotes", "RichText" );
  if ( property.isNull() ) {
    journal->setCustomProperty( "KNotes", "RichText", QLatin1String("true") );
  }

  mNoteList.insert( journal->uid(), new KNotesIconViewItem( mNotesView, journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
  KNotesIconViewItem *item = mNoteList.take( journal->uid() );
  delete item;
}

void KNotesPart::editNote( QListWidgetItem *item )
{
  if ( !mNoteEditDlg ) {
    mNoteEditDlg = new KNoteEditDlg( widget() );
  }

  Journal *journal = static_cast<KNotesIconViewItem *>( item )->journal();
  mNoteEditDlg->setTitle( journal->summary() );
  mNoteEditDlg->setText( journal->description() );

  mNoteEditDlg->noteEdit()->setFocus();
  if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
    static_cast<KNotesIconViewItem *>( item )->setIconText( mNoteEditDlg->title() );
    journal->setDescription( mNoteEditDlg->text() );
    mManager->save();
  }
}

void KNotesPart::editNote()
{
  if ( mNotesView->currentItem() ) {
    editNote( mNotesView->currentItem() );
  }
}

void KNotesPart::renameNote()
{
  KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesView->currentItem());

  QString oldName = knoteItem->realName();
  bool ok = false;
  QString newName =
    KInputDialog::getText( i18nc( "@title:window", "Rename Popup Note" ),
                           i18nc( "@label:textbox", "New Name:" ),
                           oldName, &ok, mNotesView );
  if ( ok && ( newName != oldName ) ) {
    knoteItem->setIconText( newName );
    mManager->save();
  }
}

void KNotesPart::slotOnCurrentChanged( )
{
  QAction *renameAction = actionCollection()->action( QLatin1String("edit_rename") );
  QAction *deleteAction = actionCollection()->action( QLatin1String("edit_delete") );
  QAction *editAction = actionCollection()->action( QLatin1String("edit_note") );
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

