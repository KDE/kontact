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

#include <qlistbox.h>

namespace KParts { class Part; }

namespace Kontact
{

class Plugin;

/**
  A @ref QListBoxPixmap Square Box with a large icon and a text
  underneath.
*/
class EntryItem : public QListBoxPixmap
{
  public:
    EntryItem( QListBox *, Kontact::Plugin * );
    ~EntryItem();

    Kontact::Plugin *plugin() const { return mPlugin; }

    /**
      sets the icon for the item.
      @param icon the icon to set
      @param group the icongroup
    */
    void setIcon( const QString& icon, KIcon::Group group = KIcon::Panel );

    /**
      returns the width of this item.
    */
    virtual int width( const QListBox * ) const;
    /**
      returns the height of this item.
    */
    virtual int height( const QListBox * ) const;

    /**
      returns the pixmap.
    */
    virtual const QPixmap *pixmap() const
    {
        return mPixmap;
    }

  protected:
    virtual void paint( QPainter *p);

  private:
    Kontact::Plugin *mPlugin;
    QPixmap *mPixmap;
    QListBox *mParent;
};

/**
  Navigation pane showing all parts relevant to the user
*/
class Navigator : public KListBox
{
    Q_OBJECT
  public:
    Navigator(QWidget *parent=0, const char *name=0);

    void addEntry( Kontact::Plugin * );

    QSize sizeHint() const;

  signals:
    void pluginActivated( Kontact::Plugin * );

  private slots:
    void slotExecuted(QListBoxItem *item);
};

class IconSidePane : public SidePaneBase
{
    Q_OBJECT
  public:
    IconSidePane( QWidget *parent, const char *name = 0 );
    ~IconSidePane();

    QString currentPluginName() const;

  public slots:
    /**
      Adds a new entry to the sidepane.
    */
    virtual void addEntry( Kontact::Plugin *plugin );

    virtual void selectPlugin( const QString &name );

  private:
    Navigator *mNavigator;
};

}

#endif

// vim: ts=2 sw=2 et
