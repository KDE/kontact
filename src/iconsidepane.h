/*
  This file is part of the KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KONTACT_ICONSIDEPANEBASE_H
#define KONTACT_ICONSIDEPANEBASE_H

#include "sidepanebase.h"

#include <klistbox.h>

class QSignalMapper;

namespace KParts { class Part; }

namespace Kontact
{

class Core;
class Plugin;
class Navigator;

enum IconViewMode { LargeIcons = 48, NormalIcons = 32, SmallIcons = 16, TextOnly = 0 };

/**
  A @see QListBoxPixmap Square Box with an optional icon and a text
  underneath.
*/
class EntryItem : public QListBoxItem
{
  public:
    EntryItem( Navigator *, Kontact::Plugin * );
    ~EntryItem();

    Kontact::Plugin *plugin() const { return mPlugin; }

    const QPixmap *pixmap() const { return &mPixmap; }

    Navigator* navigator() const;

    /**
      returns the width of this item.
    */
    virtual int width( const QListBox * ) const;
    /**
      returns the height of this item.
    */
    virtual int height( const QListBox * ) const;

  protected:
    void reloadPixmap();
    
    virtual void paint( QPainter *p );

  private:
    Kontact::Plugin *mPlugin;
    QPixmap mPixmap;
};

/**
  Navigation pane showing all parts relevant to the user
*/
class Navigator : public KListBox
{
    Q_OBJECT
  public:
    Navigator( SidePaneBase *parent = 0, const char *name = 0 );

    virtual void setSelected( QListBoxItem *, bool );

    void updatePlugins( QValueList<Kontact::Plugin*> plugins );

    QSize sizeHint() const;
    
    IconViewMode viewMode() { return mViewMode; }
    IconViewMode sizeIntToEnum(int size) const;
  signals:
    void pluginActivated( Kontact::Plugin * );
    
  protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent ( QDragMoveEvent * );
    void dropEvent( QDropEvent * );
    void resizeEvent( QResizeEvent * );

  protected slots:
    void slotExecuted( QListBoxItem * );
    void slotShowRMBMenu( QListBoxItem *, const QPoint& );
    void shortCutSelected( int );

  private:
    SidePaneBase *mSidePane;
    IconViewMode mViewMode;

    QSignalMapper *mMapper;
    QPtrList<KAction> mActions;
};

class IconSidePane : public SidePaneBase
{
    Q_OBJECT
  public:
    IconSidePane( Core *core, QWidget *parent, const char *name = 0 );
    ~IconSidePane();

  public slots:
    virtual void updatePlugins();
    virtual void selectPlugin( Kontact::Plugin* );
    virtual void selectPlugin( const QString &name );

  private:
    Navigator *mNavigator;
};

}

#endif
