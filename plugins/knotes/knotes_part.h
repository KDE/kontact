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

#ifndef KNOTE_PART_H
#define KNOTE_PART_H

#include <qmap.h>
#include <qpixmap.h>
#include <kparts/part.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/calendarresources.h>

typedef QMap<QString, QString> NotesMap;

class KAction;
class KListView;

class QListViewItem;
class QPoint;

class KNotesPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    KNotesPart( QObject *parent = 0, const char *name = 0 );
    ~KNotesPart();

    bool openFile();

  public slots:
    void newNote();

  signals:
    void noteSelected( const QString &name );
    void noteSelected( const QPixmap &pixmap );

  protected slots:
    void noteRenamed( QListViewItem *item, int col, const QString& text );
    void popupRMB( QListViewItem *item, const QPoint& pos, int );
    void removeNote();
    void removeSelectedNotes();
    void renameNote();
    void editNote( QListViewItem* item, const QPoint&, int );
    void editNote( QListViewItem* item );
    void reloadNotes();
    void slotCalendarChanged();

  private:
    bool lock();
    bool unlock();

    KCal::ResourceLocal *mResource;
    KCal::CalendarResources *mCalendar;
    KCal::CalendarResources::Ticket *mTicket;

    KAction *mActionEdit;
    KAction *mActionDelete;

    KListView *mNotesView;
    QPixmap mAppIcon;
    QPopupMenu *mPopupMenu;
};

#endif
