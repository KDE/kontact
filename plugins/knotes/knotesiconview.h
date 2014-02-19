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
#include <KListWidget>
#include <QMultiHash>
class KNoteConfig;
class KNoteDisplaySettings;
class KNotesIconView : public KListWidget
{
    Q_OBJECT
public:
    explicit KNotesIconView(KNotesPart *part, QWidget *parent );
    ~KNotesIconView();

    void addNote(const Akonadi::Item &item);

    KNotesIconViewItem *iconView(Akonadi::Item::Id id) const;
protected:
    void mousePressEvent( QMouseEvent * );

private:
    KNotesPart *m_part;
    QMultiHash<Akonadi::Item::Id, KNotesIconViewItem*> mNoteList;
};

class KNotesIconViewItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    KNotesIconViewItem(const Akonadi::Item &item, QListWidget *parent);
    ~KNotesIconViewItem();

    bool readOnly() const;
    void setReadOnly(bool b);

    void setIconText( const QString &text );
    QString realName() const;

    int tabSize() const;
    bool autoIndent() const;
    QFont textFont() const;
    bool isRichText() const;
    QString description() const;
    KNoteDisplaySettings *displayAttribute() const;
    Akonadi::Item item();

    void setChangeItem(const Akonadi::Item &item, const QSet<QByteArray> &set);

private:
    void prepare();
    void setDisplayDefaultValue();
    KNoteDisplaySettings *mDisplayAttribute;
    Akonadi::Item mItem;
    bool mReadOnly;
};

#if 0
class KNotesIconViewItem : public QListWidgetItem
{
public:
    KNotesIconViewItem( QListWidget *parent, Journal *journal );
    ~KNotesIconViewItem();

    void updateSettings();


private:
    Journal *mJournal;
    KNoteConfig *mConfig;
};
#endif

#endif // KNOTESICONVIEW_H
