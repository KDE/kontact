/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <kviewstatemaintainer.h>

namespace KontactInterface {
  class Plugin;
}

namespace Akonadi {
  class ChangeRecorder;
  class Collection;
  class EntityTreeModel;
  class ETMViewStateSaver;
}

class KCheckableProxyModel;

class QGridLayout;
class QLabel;
class QModelIndex;
class QItemSelectionModel;

class SummaryWidget : public KontactInterface::Summary
{
  Q_OBJECT

  public:
    SummaryWidget( KontactInterface::Plugin *plugin, QWidget *parent );

    int summaryHeight() const { return 1; }
    QStringList configModules() const;

  protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

  public slots:
    virtual void updateSummary( bool force );

  private slots:
    void selectFolder( const QString & );
    void slotCollectionChanged( const Akonadi::Collection& );
    void slotRowInserted( const QModelIndex & parent, int start, int end );

  private:
    void updateFolderList();
    void displayModel( const QModelIndex &, int&, const bool, QStringList );

    QList<QLabel*> mLabels;
    QGridLayout *mLayout;
    KontactInterface::Plugin *mPlugin;
    Akonadi::ChangeRecorder *mChangeRecorder;
    Akonadi::EntityTreeModel *mModel;
    KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mModelState;
    KCheckableProxyModel *mModelProxy;
    QItemSelectionModel *mSelectionModel;
    int mTimeOfLastMessageCountUpdate;
};

#endif
