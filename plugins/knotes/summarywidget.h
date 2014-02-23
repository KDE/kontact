/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2014 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <KontactInterface/Summary>
#include <KViewStateMaintainer>
#include <Akonadi/Item>
class QGridLayout;
class QItemSelectionModel;
class QLabel;
namespace KontactInterface {
class Plugin;
}

namespace Akonadi {
class ChangeRecorder;
class Collection;
class EntityTreeModel;
class ETMViewStateSaver;
class Item;
}
namespace NoteShared {
class NotesChangeRecorder;
class NotesAkonadiTreeModel;
}
class KCheckableProxyModel;

class KNotesSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT
public:
    KNotesSummaryWidget(KontactInterface::Plugin *plugin, QWidget *parent );
    ~KNotesSummaryWidget();

    void updateSummary( bool force = false );
    QStringList configModules() const;

protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

protected slots:
    void urlClicked( const QString & );


private slots:
    void slotCollectionChanged(const Akonadi::Collection &col);
    void slotRowInserted(const QModelIndex &parent, int start, int end);
    void updateFolderList();

    void slotItemRemoved(const Akonadi::Item &item);
    void slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &set);
private:
    QGridLayout *mLayout;
    KontactInterface::Plugin *mPlugin;
    QList<QLabel *> mLabels;
    QPixmap mPixmap;
    NoteShared::NotesChangeRecorder *mNoteRecorder;
    NoteShared::NotesAkonadiTreeModel *mNoteTreeModel;
    QItemSelectionModel *mSelectionModel;
    KCheckableProxyModel *mModelProxy;
    KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mModelState;
};

#endif
