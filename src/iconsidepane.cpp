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

#include "plugin.h"

#include "iconsidepane.h"

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

  return w + 18;
}

int EntryItem::height( const QListBox *listbox) const
{
  int h;
  if ( text().isEmpty() )
    h =  mPixmap.height();
  else 
    h = mPixmap.height() + listbox->fontMetrics().lineSpacing();

  return h + 4;
}

void EntryItem::paint( QPainter *p )
{
  QListBox *box = listBox();
  int w = box->viewport()->width();
  int y = 2;

  if ( !mPixmap.isNull() ) {
    int x = ( w - mPixmap.width() ) / 2;
    p->drawPixmap( x, y, mPixmap );
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = p->fontMetrics();
    y += mPixmap.height() + fm.height() - fm.descent();
    int x = ( w - fm.width( text() ) ) / 2;
    p->drawText( x, y, text() );
  }
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    qDrawShadePanel( p, 1, 0, w - 2, height( box ),
                     box->colorGroup(), true, 1, 0 );
  }
}

Navigator::Navigator( SidePaneBase *parent, const char *name)
  : KListBox( parent, name ), mSidePane( parent )
{
  setSelectionMode( KListBox::Single );
  QPalette pal = palette();
  QColor gray = pal.color(QPalette::Normal, QColorGroup::Mid );
  pal.setColor( QPalette::Normal, QColorGroup::Base, gray );
  pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );

  setPalette( pal );
  viewport()->setBackgroundMode( PaletteMid );

  setHScrollBarMode( QScrollView::AlwaysOff );

  setAcceptDrops( true );

  connect( this, SIGNAL( currentChanged( QListBoxItem * ) ),
           SLOT( slotExecuted( QListBoxItem * ) ) );
}

QSize Navigator::sizeHint() const
{
  return QSize( 100, 100 );
}

void Navigator::updatePlugins( QPtrList<Kontact::Plugin> plugins )
{
  clear();

  int minWidth = 0;
  for ( Kontact::Plugin *plugin = plugins.first(); plugin; plugin = plugins.next() ) {
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

void Navigator::dragEnterEvent( QDragEnterEvent *event )
{
  kdDebug() << "Navigator::dragEnterEvent()" << endl;

  dragMoveEvent( event );
}

void Navigator::dragMoveEvent( QDragMoveEvent *event )
{
  kdDebug() << "Navigator::dragEnterEvent()" << endl;
  
  kdDebug() << "  Format: " << event->format() << endl;

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    event->accept( false );
    return;
  }

  EntryItem *entry = static_cast<EntryItem *>( item );
  
  kdDebug() << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  event->accept( entry->plugin()->canDecodeDrag( event ) );
}

void Navigator::dropEvent( QDropEvent *event )
{
  kdDebug() << "Navigator::dropEvent()" << endl;

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem *>( item );
  
  kdDebug() << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  // Make sure the part is loaded before calling processDropEvent.
  // Maybe it's better, if that would be done by the plugin.
  // Hmm, this doesn't work anyway.
  mSidePane->selectPlugin( entry->plugin() );

  entry->plugin()->processDropEvent( event );  
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
