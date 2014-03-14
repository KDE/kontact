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
    QHash<Akonadi::Item::Id, KNotesIconViewItem*> noteList() const;
protected:
    void mousePressEvent( QMouseEvent * );

private:
    KNotesPart *m_part;
    QHash<Akonadi::Item::Id, KNotesIconViewItem*> mNoteList;
};

class KNotesIconViewItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    KNotesIconViewItem(const Akonadi::Item &item, QListWidget *parent);
    ~KNotesIconViewItem();

    bool readOnly() const;
    void setReadOnly(bool b, bool save = true);

    void setIconText(const QString &text , bool save = true);
    QString realName() const;

    int tabSize() const;
    bool autoIndent() const;
    QFont textFont() const;
    bool isRichText() const;
    QString description() const;
    void setDescription(const QString &);
    KNoteDisplaySettings *displayAttribute() const;
    Akonadi::Item item();

    void setChangeItem(const Akonadi::Item &item, const QSet<QByteArray> &set);
    void saveNoteContent(const QString &subject = QString(), const QString &description = QString());
    void updateSettings();
    void setChangeIconTextAndDescription(const QString &iconText, const QString &description);
private slots:
    void slotNoteSaved(KJob *job);
private:
    void prepare();
    void setDisplayDefaultValue();
    QPixmap mDefaultPixmap;

    Akonadi::Item mItem;
    KNoteDisplaySettings *mDisplayAttribute;
    bool mReadOnly;
};

#endif // KNOTESICONVIEW_H
