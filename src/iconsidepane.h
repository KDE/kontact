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

enum IconViewMode { LargeIcons = 48, NormalIcons = 32, SmallIcons = 22, ShowText = 3, ShowIcons = 5 };

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

    void setHover( bool );
    void setPaintActive( bool );
    bool paintActive() const { return mPaintActive; }
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
    bool mHasHover;
    bool mPaintActive;
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

    void highlightItem( EntryItem* item );

    IconViewMode viewMode() { return mViewMode; }
    IconViewMode sizeIntToEnum(int size) const;
    const QPtrList<KAction> & actions() { return mActions; }
    bool showIcons() const { return mShowIcons; }
    bool showText() const { return mShowText; }
  signals:
    void pluginActivated( Kontact::Plugin * );

  protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent ( QDragMoveEvent * );
    void dropEvent( QDropEvent * );
    void resizeEvent( QResizeEvent * );
    void enterEvent( QEvent* );
    void leaveEvent( QEvent* );

    void setHoverItem( QListBoxItem*, bool );
    void setPaintActiveItem( QListBoxItem*, bool );

  protected slots:
    void slotExecuted( QListBoxItem * );
    void slotMouseOn( QListBoxItem *item );
    void slotMouseOff();
    void slotShowRMBMenu( QListBoxItem *, const QPoint& );
    void shortCutSelected( int );
    void slotStopHighlight();

  private:
    SidePaneBase *mSidePane;
    IconViewMode mViewMode;

    QListBoxItem* mMouseOn;

    EntryItem*    mHighlightItem;

    QSignalMapper *mMapper;
    QPtrList<KAction> mActions;
    bool mShowIcons;
    bool mShowText;
};

class IconSidePane : public SidePaneBase
{
    Q_OBJECT
  public:
    IconSidePane( Core *core, QWidget *parent, const char *name = 0 );
    ~IconSidePane();

    virtual void indicateForegrunding( Kontact::Plugin* );

  public slots:
    virtual void updatePlugins();
    virtual void selectPlugin( Kontact::Plugin* );
    virtual void selectPlugin( const QString &name );
    const QPtrList<KAction> & actions() { return mNavigator->actions(); }

  private:
    Navigator *mNavigator;
};

}

#endif
