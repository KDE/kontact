/*
   This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
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

#ifndef KNOTES_PART_H
#define KNOTES_PART_H

#include <qdict.h>

#include <libkdepim/part.h>

#include "knotes/KNotesIface.h"

class KIconView;
class QIconViewItem;
class KNotesIconViewItem;
class KNotesResourceManager;

namespace KCal {
    class Journal;
}

typedef QMap<QString, QString> NotesMap;


class KNotesPart : public KPIM::Part, virtual public KNotesIface
{
    Q_OBJECT
public:
    KNotesPart( QObject *parent = 0, const char *name = 0 );
   ~KNotesPart();

    bool openFile();

public slots:
    QString newNote( const QString& name = QString::null,
                     const QString& text = QString::null );
    QString newNoteFromClipboard( const QString& name = QString::null );

public:
    void showNote( const QString& id ) const;
    void hideNote( const QString& id ) const;

    void killNote( const QString& id );
    void killNote( const QString& id, bool force );

    QString name( const QString& id ) const;
    QString text( const QString& id ) const;

    void setName( const QString& id, const QString& newName );
    void setText( const QString& id, const QString& newText );

    QMap<QString, QString> notes() const;

// TODO: remove for KDE 4.0
    void sync( const QString& app );
    bool isNew( const QString& app, const QString& id ) const;
    bool isModified( const QString& app, const QString& id ) const;

// signals:
//     void noteSelected( const QString &name );
//     void noteSelected( const QPixmap &pixmap );

private slots:
    void createNote( KCal::Journal *journal );
    void killNote( KCal::Journal *journal );

    void popupRMB( QIconViewItem *item, const QPoint& pos );
    void killSelectedNotes();

private:
    KIconView  *m_notesView;
    QPopupMenu *m_context_menu;

    KNotesResourceManager     *m_manager;
    QDict<KNotesIconViewItem>  m_noteList;
};


#endif
