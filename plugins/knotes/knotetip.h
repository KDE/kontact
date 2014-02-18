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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

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

#ifndef KNOTETIP_H
#define KNOTETIP_H

#include <QFrame>

class KNotesIconViewItem;

class QTextEdit;
class QListWidget;

class KNoteTip : public QFrame
{
public:
    explicit KNoteTip( QListWidget *parent );
    ~KNoteTip();

    void setNote( KNotesIconViewItem *item );

protected:
    bool eventFilter( QObject *, QEvent *e );
    void timerEvent( QTimerEvent * );
    void resizeEvent( QResizeEvent * );

private:
    void setColor( const QColor &fg, const QColor &bg );
    void setFilter( bool enable );
    void reposition();

private:
    bool mFilter;
    QListWidget *mView;
    KNotesIconViewItem *mNoteIVI;
    QTextEdit *mPreview;
};

#endif
