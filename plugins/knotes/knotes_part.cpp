/*
   This file is part of the KDE project
   Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2004 Michael Brade <brade@kde.org>

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
#include <qclipboard.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kiconview.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>

#include <libkcal/journal.h>

#include <libkdepim/infoextension.h>
#include <libkdepim/sidebarextension.h>

#include "knotes/resourcemanager.h"
#include "knotes_part.h"


class KNotesIconViewItem : public KIconViewItem
{
public:
    KNotesIconViewItem( KIconView *parent, KCal::Journal *journal )
      : KIconViewItem( parent ),
        m_journal( journal )
    {
        setRenameEnabled( true );

        setPixmap( KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Desktop ) );
        setText( journal->summary() );
    }

    KCal::Journal *journal()
    {
        return m_journal;
    }

    virtual void setText( const QString & text )
    {
        KIconViewItem::setText( text );
        m_journal->setSummary( text );
    }

    virtual QString text() const
    {
        return m_journal->summary();
    }

private:
    KCal::Journal *m_journal;
};


KNotesPart::KNotesPart( QObject *parent, const char *name )
  : DCOPObject("KNotesIface"), KPIM::Part( parent, name ),
    m_context_menu( 0 )
{
    m_noteList.setAutoDelete( true );

    setInstance( new KInstance( "knotes" ) );

    // create the actions
    new KAction( i18n("&New..."), "knotes", CTRL+Key_N, this, SLOT(newNote()),
                 actionCollection(), "file_new" );
    new KAction( i18n("Rename"), "text", this, SLOT(renameNote()),
                 actionCollection(), "edit_rename" );
    new KAction( i18n("Delete"), "editdelete", 0, this, SLOT(killSelectedNotes()),
                 actionCollection(), "edit_delete" );

    // TODO styleguide: s/New.../New/, s/Rename/Rename.../
    // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

    // create the view
    m_notesView = new KIconView();
    m_notesView->setSelectionMode( QIconView::Extended );
    m_notesView->setItemsMovable( false );
    m_notesView->setResizeMode( QIconView::Adjust );

//    connect( m_notesView, SIGNAL(executed( QIconViewItem * )),
//             this, SLOT(editNote( QIconViewItem * )) );
//    connect( m_notesView, SIGNAL(returnPressed( KIconViewItem * )),
//             this, SLOT(editNote( QIconViewItem * )) );
    connect( m_notesView, SIGNAL(contextMenuRequested( QIconViewItem *, const QPoint & )),
             this, SLOT(popupRMB( QIconViewItem *, const QPoint & )) );

    setWidget( m_notesView );

    new KParts::SideBarExtension( m_notesView, this, "NotesSideBarExtension" );

//     KParts::InfoExtension *info = new KParts::InfoExtension( this, "NotesInfoExtension" );
//     connect( this, SIGNAL(noteSelected( const QString& )),
//              info, SIGNAL(textChanged( const QString& )) );
//     connect( this, SIGNAL(noteSelected( const QPixmap& )),
//              info, SIGNAL(iconChanged( const QPixmap& )) );


    setXMLFile( "knotes_part.rc" );

    // create the resource manager
    m_manager = new KNotesResourceManager();
    connect( m_manager, SIGNAL(sigRegisteredNote( KCal::Journal * )),
             this,      SLOT(createNote( KCal::Journal * )) );
    connect( m_manager, SIGNAL(sigDeregisteredNote( KCal::Journal * )),
             this,      SLOT(killNote( KCal::Journal * )) );

    // read the notes
    m_manager->load();
}

KNotesPart::~KNotesPart()
{
    delete m_manager;
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

    m_manager->addNewNote( journal );

    showNote( journal->uid() );

    return journal->uid();
}

QString KNotesPart::newNoteFromClipboard( const QString& name )
{
    const QString& text = KApplication::clipboard()->text();
    return newNote( name, text );
}

void KNotesPart::showNote( const QString& id ) const
{
    KNotesIconViewItem *note = m_noteList[id];
    if ( !note )
        return;

    m_notesView->ensureItemVisible( note );
    m_notesView->setCurrentItem( note );
}

void KNotesPart::hideNote( const QString& ) const
{
    // simply does nothing, there is nothing to hide
}

void KNotesPart::killNote( const QString& id )
{
    killNote( id, false );
}

void KNotesPart::killNote( const QString& id, bool force )
{
    KNotesIconViewItem *note = m_noteList[id];

    if ( note && !force && KMessageBox::warningContinueCancelList( m_notesView,
            i18n( "Do you really want to delete this note?" ),
            m_noteList[id]->text(), i18n("Confirm Delete"),
            KGuiItem( i18n("Delete"), "editdelete" ) ) == KMessageBox::Continue )
    {
        m_manager->deleteNote( m_noteList[id]->journal() );
    }
}

QString KNotesPart::name( const QString& id ) const
{
    KNotesIconViewItem *note = m_noteList[id];
    if ( note )
        return note->text();
    else
        return QString::null;
}

QString KNotesPart::text( const QString& id ) const
{
    KNotesIconViewItem *note = m_noteList[id];
    if ( note )
        return note->journal()->description();
    else
        return QString::null;
}

void KNotesPart::setName( const QString& id, const QString& newName )
{
    KNotesIconViewItem *note = m_noteList[id];
    if ( note )
        return note->setText( newName );
}

void KNotesPart::setText( const QString& id, const QString& newText )
{
    KNotesIconViewItem *note = m_noteList[id];
    if ( note )
        return note->journal()->setDescription( newText );
}

QMap<QString, QString> KNotesPart::notes() const
{
    QMap<QString, QString> notes;
    QDictIterator<KNotesIconViewItem> it( m_noteList );

    for ( ; it.current(); ++it )
        notes.insert( (*it)->journal()->uid(), (*it)->journal()->description() );

    return notes;
}

// TODO KDE 4.0: remove

void KNotesPart::sync( const QString& app )
{
}

bool KNotesPart::isNew( const QString& app, const QString& id ) const
{
    return true;
}

bool KNotesPart::isModified( const QString& app, const QString& id ) const
{
    return true;
}


// private stuff


void KNotesPart::killSelectedNotes()
{
    QPtrList<KNotesIconViewItem> items;
    QStringList notes;

    KNotesIconViewItem *knivi;
    for ( QIconViewItem *it = m_notesView->firstItem(); it; it = it->nextItem() )
    {
        if ( it->isSelected() )
        {
            knivi = static_cast<KNotesIconViewItem *>( it );
            items.append( knivi );
            notes.append( knivi->text() );
        }
    }

    if ( items.isEmpty() )
        return;

//   if ( !lock() )
//     return;

    int ret = KMessageBox::warningContinueCancelList( m_notesView,
            i18n( "Do you really want to delete this note?",
                  "Do you really want to delete these %n notes?", items.count() ),
            notes, i18n("Confirm Delete"),
            KGuiItem( i18n("Delete"), "editdelete" )
                                                    );

    if ( ret == KMessageBox::Continue )
    {
        QPtrListIterator<KNotesIconViewItem> kniviIt( items );
        while ( (knivi = *kniviIt) )
        {
            ++kniviIt;
            m_manager->deleteNote( knivi->journal() );
        }
    }

//   unlock();
}

void KNotesPart::popupRMB( QIconViewItem *item, const QPoint& pos )
{
    if ( !m_context_menu )
        m_context_menu = static_cast<QPopupMenu *>(factory()->container( "note_context", this ));

    if ( !m_context_menu || !item )
        return;

    m_context_menu->popup( pos );
}


// create and kill the icon view item corresponding to the note

void KNotesPart::createNote( KCal::Journal *journal )
{
    m_noteList.insert( journal->uid(), new KNotesIconViewItem( m_notesView, journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
    m_noteList.remove( journal->uid() );
}


#include "knotes_part.moc"
