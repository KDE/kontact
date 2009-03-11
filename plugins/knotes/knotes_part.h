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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KNOTES_PART_H
#define KNOTES_PART_H

#include <kcal/journal.h>

#include <kparts/part.h>

#include <q3dict.h>

class K3IconView;
class Q3IconViewItem;
class KNotesIconViewItem;
class KNoteTip;
class KNoteEditDlg;
class KNotesResourceManager;

namespace KCal {
  class Journal;
}

class KNotesPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    KNotesPart( QObject *parent = 0 );
    ~KNotesPart();

    bool openFile();

  public slots:
    QString newNote( const QString &name = QString(),
                     const QString &text = QString() );
    QString newNoteFromClipboard( const QString &name = QString() );

  public:
    void killNote( const QString &id );
    void killNote( const QString &id, bool force );

    QString name( const QString &id ) const;
    QString text( const QString &id ) const;

    void setName( const QString &id, const QString &newName );
    void setText( const QString &id, const QString &newText );

    QMap<QString, QString> notes() const;

  private slots:
    void createNote( KCal::Journal *journal );
    void killNote( KCal::Journal *journal );

    void editNote( Q3IconViewItem *item );

    void renameNote();
    void renamedNote( Q3IconViewItem *item );

    void slotOnItem( Q3IconViewItem *item );
    void slotOnViewport();
    void slotOnCurrentChanged( Q3IconViewItem *item );

    void popupRMB( Q3IconViewItem *item, const QPoint &pos );
    void killSelectedNotes();

    void printSelectedNotes();

  private:
    K3IconView *mNotesView;
    KNoteTip *mNoteTip;
    KNoteEditDlg *mNoteEditDlg;

    KNotesResourceManager *mManager;
    Q3Dict<KNotesIconViewItem> mNoteList;
    QString mOldName;
};

#endif
