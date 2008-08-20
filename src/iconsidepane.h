/*
  This file is part of the KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>

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

#ifndef KONTACT_ICONSIDEPANEBASE_H
#define KONTACT_ICONSIDEPANEBASE_H

#include "sidepanebase.h"
#include "prefs.h"

#include <QtGui/QListView>

namespace KParts {
  class Part;
}

class KAction;

namespace Kontact
{

class Core;
class Plugin;
class Navigator;
class Model;

class Navigator : public QListView
{
  Q_OBJECT

  public:
    explicit Navigator( SidePaneBase *parent = 0 );

    void updatePlugins( QList<Plugin*> plugins );
    void setCurrentPlugin( const QString &plugin );

    int iconSize() const
    {
      return mIconSize;
    }

    bool showIcons() const
    {
      return mShowIcons;
    }

    bool showText() const
    {
      return mShowText;
    }

    virtual QSize sizeHint() const;

  signals:
    void pluginActivated( Kontact::Plugin *plugin );

  protected:
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragMoveEvent( QDragMoveEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void showEvent( QShowEvent * event );

  private slots:
    void slotCurrentChanged( const QModelIndex &current );
    void slotActionTriggered( bool checked );
    void updateNavigatorSize();

  private:
    SidePaneBase *mSidePane;
    Model *mModel;

    int mIconSize;
    bool mShowIcons;
    bool mShowText;

    KAction *mShowIconsAction;
    KAction *mShowTextAction;
    KAction *mShowBothAction;
    KAction *mBigIconsAction;
    KAction *mNormalIconsAction;
    KAction *mSmallIconsAction;
};

class IconSidePane : public SidePaneBase
{
  Q_OBJECT

  public:
    IconSidePane( Core *core, QWidget *parent );
    ~IconSidePane();

    void setCurrentPlugin( const QString &plugin );

  public slots:
    virtual void updatePlugins();

  protected:
    virtual void resizeEvent( QResizeEvent *event );

  private:
    Navigator *mNavigator;
};

}

#endif
