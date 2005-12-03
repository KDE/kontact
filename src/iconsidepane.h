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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KONTACT_ICONSIDEPANEBASE_H
#define KONTACT_ICONSIDEPANEBASE_H

#include <qtooltip.h>
//Added by qt3to4:
#include <QPixmap>
#include <QList>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>

#include <klistbox.h>

#include "sidepanebase.h"
#include "prefs.h"


class QSignalMapper;

namespace KParts { class Part; }

namespace Kontact
{

class Core;
class Plugin;
class Navigator;

enum IconViewMode { LargeIcons = 48, NormalIcons = 32, SmallIcons = 22, ShowText = 3, ShowIcons = 5 };


/**
  A QListBoxPixmap Square Box with an optional icon and a text
  underneath.
*/
class EntryItem : public Q3ListBoxItem
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
    virtual int width( const Q3ListBox * ) const;
    /**
      returns the height of this item.
    */
    virtual int height( const Q3ListBox * ) const;

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
 * Tooltip that changes text depending on the item it is above.
 * Compliments of "Practical Qt" by Dalheimer, Petersen et al.
 */
#warning Port me!
#if 0
class EntryItemToolTip : public QToolTip
{
  public:
    EntryItemToolTip( Q3ListBox* parent )
      : QToolTip( parent->viewport() ), mListBox( parent )
      {}
  protected:
    void maybeTip( const QPoint& p ) {
      // We only show tooltips when there are no texts shown
      if ( Prefs::self()->sidePaneShowText() ) return;
      if ( !mListBox ) return;
      Q3ListBoxItem* item = mListBox->itemAt( p );
      if ( !item ) return;
      const QRect itemRect = mListBox->itemRect( item );
      if ( !itemRect.isValid() ) return;

      const EntryItem *entryItem = static_cast<EntryItem*>( item );
      QString tipStr = entryItem->text();
      tip( itemRect, tipStr );
    }
  private:
    Q3ListBox* mListBox;
};
#endif

/**
  Navigation pane showing all parts relevant to the user
*/
class Navigator : public KListBox
{
    Q_OBJECT
  public:
    Navigator( SidePaneBase *parent = 0, const char *name = 0 );

    virtual void setSelected( Q3ListBoxItem *, bool );

    void updatePlugins( QList<Kontact::Plugin*> plugins );

    QSize sizeHint() const;

    void highlightItem( EntryItem* item );

    IconViewMode viewMode() { return mViewMode; }
    IconViewMode sizeIntToEnum(int size) const;
    const QList<KAction*> & actions() { return mActions; }
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

    void setHoverItem( Q3ListBoxItem*, bool );
    void setPaintActiveItem( Q3ListBoxItem*, bool );

  protected slots:
    void slotExecuted( Q3ListBoxItem * );
    void slotMouseOn( Q3ListBoxItem *item );
    void slotMouseOff();
    void slotShowRMBMenu( Q3ListBoxItem *, const QPoint& );
    void shortCutSelected( int );
    void slotStopHighlight();

  private:
    SidePaneBase *mSidePane;
    IconViewMode mViewMode;

    Q3ListBoxItem* mMouseOn;

    EntryItem*    mHighlightItem;

    QSignalMapper *mMapper;
    QList<KAction*> mActions;
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
    const QList<KAction*> & actions() { return mNavigator->actions(); }

  private:
    Navigator *mNavigator;
};

}

#endif
