/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2013 Laurent Montel <montel@kde.org>

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

#ifndef KCMKMAILSUMMARY_H
#define KCMKMAILSUMMARY_H

#include <KCModule>
#include <KViewStateMaintainer>

namespace Akonadi {
  class EntityTreeModel;
  class ChangeRecorder;
  class ETMViewStateSaver;
}
class KRecursiveFilterProxyModel;
class KCheckableProxyModel;

class QCheckBox;
class QTreeView;
class QItemSelectionModel;

class KCMKMailSummary : public KCModule
{
  Q_OBJECT

  public:
    explicit KCMKMailSummary( const KComponentData &inst, QWidget *parent = 0 );

    virtual void load();
    virtual void save();
    virtual void defaults();

  private slots:
    void modified();
    void slotSetCollectionFilter(const QString &filter);

  private:
    void initGUI();
    void initFolders();
    void loadFolders();
    void storeFolders();

    QTreeView *mFolderView;
    QCheckBox *mFullPath;
    QItemSelectionModel *mSelectionModel;
    Akonadi::EntityTreeModel *mModel;
    Akonadi::ChangeRecorder *mChangeRecorder;
    KCheckableProxyModel *mCheckProxy;
    KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mModelState;
    KRecursiveFilterProxyModel *mCollectionFilter;
};

#endif
