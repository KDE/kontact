/*
  This file is part of KDE Kontact.

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

#include <qptrlist.h>
#include <qwidgetstack.h>
#include <qsignal.h>
#include <qobjectlist.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qfontmetrics.h>
#include <qstyle.h>
#include <qframe.h>
#include <qdrawutil.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "kpplugin.h"

#include "iconsidepane.h"

using namespace Kontact;

EntryItem::EntryItem( QListBox *parent, Kontact::Plugin *plugin )
  : QListBoxPixmap( KIconLoader::unknown() ),
    mPlugin( plugin ),
    mParent( parent )
{
  QPixmap pixmap = KGlobal::iconLoader()->loadIcon( plugin->icon(),
                                                    KIcon::Desktop, 48 );
  mPixmap = new QPixmap( pixmap );
  setCustomHighlighting( true );
  setText( plugin->pluginName() );
}

EntryItem::~EntryItem()
{
}

int EntryItem::width( const QListBox *listbox) const
{
  return listbox->viewport()->width();
}

int EntryItem::height( const QListBox *listbox) const
{
  int min = 0;
  min = listbox->fontMetrics().lineSpacing() + pixmap()->height() + 6;
  return min;
}

void EntryItem::paint( QPainter *p )
{
  QListBox *box = listBox();
  int w = width( box );
  static const int margin = 3;
  int y = margin;
  const QPixmap *pm = pixmap();

  if ( !pm->isNull() ) {
    int x = (w - pm->width()) / 2;
    x = QMAX( x, margin );
    p->drawPixmap( x, y, *pm );
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = p->fontMetrics();
    y += pm->height() + fm.height() - fm.descent();
    int x = (w - fm.width( text() )) / 2;
    x = QMAX( x, margin );
    p->drawText( x, y, text() );
  }
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    qDrawShadePanel( p, 1, 0, w -2, height(box),
                     box->colorGroup(), true, 1, 0 );
  }
}


Navigator::Navigator(QWidget *parent, const char *name)
  : KListBox(parent, name)
{
  setSelectionMode( KListBox::Single );
  QPalette pal = palette();
  QColor gray = pal.color(QPalette::Normal, QColorGroup::Mid );
  pal.setColor( QPalette::Normal, QColorGroup::Base, gray );
  pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );

  setPalette( pal );
  viewport()->setBackgroundMode( PaletteMid );

  setHScrollBarMode( QScrollView::AlwaysOff );

  connect( this, SIGNAL( currentChanged( QListBoxItem * ) ),
           SLOT( slotExecuted( QListBoxItem * ) ) );
}

QSize Navigator::sizeHint() const
{
  return QSize( 100, 100 );
}

void Navigator::addEntry( Kontact::Plugin *plugin )
{
  insertItem( new EntryItem( this, plugin ) );
}

void Navigator::slotExecuted( QListBoxItem *item )
{
  if ( !item ) return;
  
  EntryItem *entry = static_cast<EntryItem *>( item );

  emit pluginActivated( entry->plugin() );
}


IconSidePane::IconSidePane( QWidget *parent, const char *name )
  : SidePaneBase( parent, name )
{
  mNavigator = new Navigator( this );
  connect( mNavigator, SIGNAL( pluginActivated( Kontact::Plugin * ) ),
           SIGNAL( showPart( Kontact::Plugin * ) ) );
}

IconSidePane::~IconSidePane()
{
}

void IconSidePane::addEntry( Kontact::Plugin *plugin )
{
  kdDebug() << "IconSidePane::addEntry()" << endl;

  mNavigator->addEntry( plugin );
}

void IconSidePane::selectPlugin( const QString &name )
{
  uint i;
  for( i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem *>( mNavigator->item( i ) );
    if ( item->plugin()->name() == name ) {
      mNavigator->setCurrentItem( i );
      break;
    }
  }
}

QString IconSidePane::currentPluginName() const
{
  QListBoxItem *i = mNavigator->item( mNavigator->currentItem() );
  if ( !i ) return QString::null;
  
  EntryItem *item = static_cast<EntryItem *>( i );
  
  return item->plugin()->name();
}

#include "iconsidepane.moc"

// vim: ts=2 sw=2 et
