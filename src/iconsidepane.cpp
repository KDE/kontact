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
#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>
#include <kiconeffect.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "plugin.h"

#include "iconsidepane.h"

#include "tiles.h"

extern bool qt_use_xrender;
extern bool qt_has_xft;

using namespace Kontact;

EntryItem::EntryItem( QListBox *parent, Kontact::Plugin *plugin )
  : QListBoxItem( parent ),
    mPlugin( plugin )
{
  mPixmap = KGlobal::iconLoader()->loadIcon( plugin->icon(),
                                             KIcon::Desktop, 48 );
  setCustomHighlighting( true );
  setText( plugin->title() );
}

EntryItem::~EntryItem()
{
}

int EntryItem::width( const QListBox *listbox) const
{
  int w;
  if ( text().isEmpty() )
    w = mPixmap.width();
  else
    w = QMAX( mPixmap.width(), listbox->fontMetrics().width( text() ) );

  return w + 22;
}

int EntryItem::height( const QListBox *listbox) const
{
  int h;
  if ( text().isEmpty() )
    h =  mPixmap.height();
  else 
    h = mPixmap.height() + listbox->fontMetrics().lineSpacing();

  return h + 12;
}

void EntryItem::paint( QPainter *p )
{
  Navigator *box = static_cast<Navigator*>(listBox());
  int w = box->viewport()->width();
  int h = height( box );
  int y = 2;
  QPixmap pix( w, h );
  pix.fill( box->viewport()->paletteBackgroundColor() );
  QPainter dp;
  dp.begin( &pix );
  QColor save = dp.pen().color();
  QColorGroup group = box->colorGroup();
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    if (qt_use_xrender && qt_has_xft ) {
      dp.drawPixmap( 0, 0, *box->tile(TopLeft) );
      dp.drawTiledPixmap( 10, 0, w-20, 10, *box->tile(Top) );
      dp.drawPixmap( w - 10, 0, *box->tile(TopRight) );

      dp.drawTiledPixmap( 0, 10, 10, h-20, *box->tile(Left) );
      dp.drawTiledPixmap( 10, 10, w-20, h-20, *box->tile(Center) );
      dp.drawTiledPixmap( w-10, 10, 10, h-20, *box->tile(Right) );

      dp.drawPixmap( 0, h-10, *box->tile(BottomLeft) );
      dp.drawTiledPixmap( 10, h-10, w-20, 10, *box->tile(Bottom) );
      dp.drawPixmap( w-10, h-10, *box->tile(BottomRight) );
    }
    else {
      dp.setPen( Qt::darkGray );
      dp.setBrush( Qt::darkGray );
      dp.drawRoundRect( 0, 0, w, h, 35, 35 );
    }
 }

  dp.setPen( save );
  
  if ( !mPixmap.isNull() ) {
    int x = ( w - mPixmap.width() ) / 2;
    QImage img = mPixmap.convertToImage();
    if ( box->mMouseOverItem == this && 
        !( isCurrent() || isSelected() ) )
      KIconEffect::toGamma( img, 0.8f );
    dp.drawPixmap( x, y, img );
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = dp.fontMetrics();
    y += mPixmap.height() + fm.height() - fm.descent();
    int x = ( w - fm.width( text() ) ) / 2;
    if ( isCurrent() || isSelected() ) {
      dp.setPen( group.shadow() );
      dp.drawText( x+1, y+1, text() );
      dp.setPen( group.brightText() );
    }
    dp.drawText( x, y, text() );
  }
  dp.end();
  p->drawPixmap( 0, 0, pix );
}

Navigator::Navigator( SidePaneBase *parent, const char *name)
  : KListBox( parent, name ), mSidePane( parent ), mMouseOverItem( 0 )
{
  setSelectionMode( KListBox::Single );
  viewport()->setBackgroundMode( PaletteMid );
  setHScrollBarMode( QScrollView::AlwaysOff );
  setAcceptDrops( true );
  setCursor( KCursor::handCursor() );

  connect( this, SIGNAL( currentChanged( QListBoxItem * ) ),
           SLOT( slotExecuted( QListBoxItem * ) ) );

  KontactImageDb *imagedb = KontactImageDb::instance();

  QStringList images;
  images << "left" << "topleft" << "top" << "topright" << "right"
		  << "bottomright" << "bottom" << "bottomleft" << "center";
#if 1
  for (int i = 0; i < NTiles; i++)  
    tiles[i] = new QPixmap( *imagedb->image( images[i] ) );
#endif
}

QSize Navigator::sizeHint() const
{
  return QSize( 100, 100 );
}

void Navigator::setSelected( QListBoxItem *i, bool sel )
{
  // Reimplmemented to avoid the immediate activation of
  // the item. might turn out it doesn't work, we check that
  // an confirm from MainWindow::selectPlugin()
  if (sel) {
    EntryItem *entry = static_cast<EntryItem *>( i );
    emit pluginActivated( entry->plugin() );
  }
}

void Navigator::updatePlugins( QValueList<Kontact::Plugin*> plugins )
{
  clear();

  int minWidth = 0;
  QValueList<Kontact::Plugin*>::ConstIterator end = plugins.end();
  QValueList<Kontact::Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = *it;
    if ( !plugin->showInSideBar() )
      continue;

    EntryItem *item = new EntryItem( this, plugin );

    if ( item->width( this ) > minWidth )
      minWidth = item->width( this );
  }

  parentWidget()->setFixedWidth( minWidth );
}

void Navigator::slotExecuted( QListBoxItem *item )
{
  if ( !item ) return;
  
  EntryItem *entry = static_cast<EntryItem *>( item );

  emit pluginActivated( entry->plugin() );
}

void Navigator::mouseMoveEvent( QMouseEvent *event )
{
  EntryItem *i = static_cast<EntryItem*>(itemAt(event->pos()));
  if ( mMouseOverItem && mMouseOverItem != i )
  {
    EntryItem *tmp = mMouseOverItem;
    mMouseOverItem = 0;
    updateItem(tmp);
  }

  if ( i )
  {
    if ( i != mMouseOverItem ) {
      mMouseOverItem = i;
      updateItem(mMouseOverItem);
    }
  }
}

void Navigator::dragEnterEvent( QDragEnterEvent *event )
{
  kdDebug(5600) << "Navigator::dragEnterEvent()" << endl;

  dragMoveEvent( event );
}

void Navigator::dragMoveEvent( QDragMoveEvent *event )
{
  kdDebug(5600) << "Navigator::dragEnterEvent()" << endl;
  
  kdDebug(5600) << "  Format: " << event->format() << endl;

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    event->accept( false );
    return;
  }

  EntryItem *entry = static_cast<EntryItem *>( item );
  
  kdDebug(5600) << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  event->accept( entry->plugin()->canDecodeDrag( event ) );
}

void Navigator::dropEvent( QDropEvent *event )
{
  kdDebug(5600) << "Navigator::dropEvent()" << endl;

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem *>( item );
  
  kdDebug(5600) << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  entry->plugin()->processDropEvent( event );  
}

void Navigator::resizeEvent( QResizeEvent *event )
{
  QListBox::resizeEvent( event );
  triggerUpdate( true );
}

IconSidePane::IconSidePane( Core *core, QWidget *parent, const char *name )
  : SidePaneBase( core, parent, name )
{
  mNavigator = new Navigator( this );
  connect( mNavigator, SIGNAL( pluginActivated( Kontact::Plugin * ) ),
           SIGNAL( pluginSelected( Kontact::Plugin * ) ) );

  setAcceptDrops( true );
}

IconSidePane::~IconSidePane()
{
}

void IconSidePane::updatePlugins()
{
  mNavigator->updatePlugins( core()->pluginList() );
}

void IconSidePane::selectPlugin( Kontact::Plugin *plugin )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  uint i;
  for ( i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem *>( mNavigator->item( i ) );
    if ( item->plugin() == plugin ) {
      mNavigator->setCurrentItem( i );
      break;
    }
  }

  blockSignals( blocked );
}

void IconSidePane::selectPlugin( const QString &name )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  uint i;
  for ( i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem *>( mNavigator->item( i ) );
    if ( item->plugin()->identifier() == name ) {
      mNavigator->setCurrentItem( i );
      break;
    }
  }

  blockSignals( blocked );
}

#include "iconsidepane.moc"

// vim: sw=2 sts=2 et tw=80
