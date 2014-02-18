/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KNOTESICONVIEW_H
#define KNOTESICONVIEW_H

#include "knotes_part.h"
#include <QListWidget>
class KNoteConfig;
class KNotesIconView : public QListWidget
{
    Q_OBJECT
public:
    explicit KNotesIconView(QWidget *parent );
    ~KNotesIconView();

    void addNote();

protected:
    void mousePressEvent( QMouseEvent * );

private:
    //KNotesPart *m_part;
};

class KNotesIconViewItem : public QListWidgetItem
{
public:
    KNotesIconViewItem( QListWidget *parent);
    ~KNotesIconViewItem();
};

#if 0
class KNotesIconViewItem : public QListWidgetItem
{
public:
    KNotesIconViewItem( QListWidget *parent, Journal *journal );
    ~KNotesIconViewItem();

    Journal *journal() const;
    QString realName() const;
    void setIconText( const QString &text );
    KNoteConfig *config();
    void updateSettings();
    bool readOnly() const;
    void setReadOnly(bool b);
    int tabSize() const;

    bool autoIndent() const;
    bool isRichText() const;

    QFont textFont() const;

private:
    Journal *mJournal;
    KNoteConfig *mConfig;
};
#endif

#endif // KNOTESICONVIEW_H
