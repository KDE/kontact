/*
   This file is part of the KDE project
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

   In addition, as a special exception, the copyright holders give
   permission to link the code of this program with any edition of
   the Qt library by Trolltech AS, Norway (or with modified versions
   of Qt that use the same license as Qt), and distribute linked
   combinations including the two.  You must obey the GNU General
   Public License in all respects for all of the code used other than
   Qt.  If you modify this file, you may extend this exception to
   your version of the file, but you are not obligated to do so.  If
   you do not wish to do so, delete this exception statement from
   your version.
*/

#ifndef KNOTES_PART_P_H
#define KNOTES_PART_P_H

#include <qlayout.h>

#include <kactioncollection.h>
#include <klocale.h>
#include <kiconview.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kxmlguibuilder.h>

#include <libkcal/journal.h>

#include "knotes/knoteedit.h"


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

private:
    KCal::Journal *m_journal;
};


class KNoteEditDlg : public KDialogBase, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    KNoteEditDlg( QWidget *parent = 0, const char *name = 0 )
        : KDialogBase( Plain, i18n("Edit Note"), Ok|Cancel, Ok, parent, name, true, true )
    {
        // this dialog is modal to prevent one from editing the same note twice in two
        // different windows

        setInstance( new KInstance( "knotes" ) ); // TODO: hm, memleak??
        setXMLFile( "knotesui.rc" );
        actionCollection()->setWidget( this );

        QWidget *page = plainPage();
        QVBoxLayout *layout = new QVBoxLayout( page );

        m_noteEdit = new KNoteEdit( actionCollection(), page );
        m_noteEdit->setFocus();

        KXMLGUIBuilder builder( page );
        KXMLGUIFactory factory( &builder, this );
        factory.addClient( this );

        m_tool = static_cast<KToolBar *>(factory.container( "note_tool", this ));

        layout->addWidget( m_tool );
        layout->addWidget( m_noteEdit );
    }

    QString text() const
    {
        return m_noteEdit->text();
    }

    void setText( const QString& text )
    {
        m_noteEdit->setText( text );
    }

private:
    KNoteEdit  *m_noteEdit;
    KToolBar   *m_tool;
    KPopupMenu *m_edit_menu;
};


#endif
