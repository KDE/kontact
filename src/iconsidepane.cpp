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
#include <qsignalmapper.h>
#include <qstyle.h>
#include <qframe.h>
#include <qdrawutil.h>

#include <kpopupmenu.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "plugin.h"

#include "prefs.h"
#include "iconsidepane.h"

namespace Kontact
{

//ugly wrapper class for adding an operator< to the Plugin class

class PluginProxy
{
public:
  PluginProxy()
    : m_plugin( 0 )
  { }

  PluginProxy( Plugin * plugin )
    : m_plugin( plugin )
  { }

  PluginProxy & operator=( Plugin * plugin )
  {
    m_plugin = plugin;
    return *this;
  }

  bool operator<( PluginProxy & rhs ) const
  {
    return m_plugin->weight() < rhs.m_plugin->weight();
  }

  Plugin * plugin() const
  {
    return m_plugin;
  }

private:
  Plugin * m_plugin;
};
} //namespace

using namespace Kontact;

EntryItem::EntryItem( Navigator *parent, Kontact::Plugin *plugin )
  : QListBoxItem( parent ),
    mPlugin( plugin )
{
  reloadPixmap();
  setCustomHighlighting( true );
  setText( plugin->title() );
}

EntryItem::~EntryItem()
{
}

void EntryItem::reloadPixmap()
{
  int size = (int)navigator()->viewMode();
  if (size != 0)
    mPixmap = KGlobal::iconLoader()->loadIcon( mPlugin->icon(),
  KIcon::Desktop, size );
  else
    mPixmap = QPixmap();
}

Navigator* EntryItem::navigator() const
{ 
  return static_cast<Navigator*>(listBox());
}

int EntryItem::width( const QListBox *listbox) const
{
  int w;
  if ( text().isEmpty() )
    w = mPixmap.width();
  else
    w = QMAX( (int)navigator()->viewMode(), listbox->fontMetrics().width( text() ) );

  return w + 36;
}

int EntryItem::height( const QListBox *listbox) const
{
  int h;
  if ( text().isEmpty() )
    h =  mPixmap.height();
  else 
    h = (int)navigator()->viewMode() + listbox->fontMetrics().lineSpacing();

  return h + 4;
}

void EntryItem::paint( QPainter *p )
{
  reloadPixmap();
  
  QListBox *box = listBox();
  int w = box->viewport()->width();
  int y = 2;

  if ( !mPixmap.isNull() ) {
    int x = ( w - mPixmap.width() ) / 2;
    p->drawPixmap( x, y, mPixmap );
  }

  QColor save;
  if ( isCurrent() || isSelected() ) {
    save = p->pen().color();
    p->setPen(listBox()->colorGroup().brightText());
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = p->fontMetrics();
    y += mPixmap.height() + fm.height() - fm.descent();
    int x = ( w - fm.width( text() ) ) / 2;
    p->drawText( x, y, text() );
  }
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    p->setPen(save);
    QColorGroup group = box->colorGroup();
    group.setColor( QColorGroup::Dark, Qt::black );
    qDrawShadePanel( p, 1, 0, w - 2, height( box ),
                     group, true, 1, 0 );
  }
}

Navigator::Navigator( SidePaneBase *parent, const char *name)
  : KListBox( parent, name ), mSidePane( parent )
{
  mViewMode = sizeIntToEnum( Prefs::self()->sidePaneIconSize() );
  setSelectionMode( KListBox::Single );
  viewport()->setBackgroundMode( PaletteMid );
  setHScrollBarMode( QScrollView::AlwaysOff );
  setAcceptDrops( true );

  connect( this, SIGNAL( selectionChanged( QListBoxItem * ) ),
           SLOT( slotExecuted( QListBoxItem * ) ) );

  connect( this, SIGNAL( rightButtonPressed( QListBoxItem *, const QPoint& ) ),
           SLOT( slotShowRMBMenu( QListBoxItem *, const QPoint& ) ) );

  mMapper = new QSignalMapper( this );
  connect( mMapper, SIGNAL( mapped( int ) ), SLOT( shortCutSelected( int ) ) );
}

QSize Navigator::sizeHint() const
{
  return QSize( 100, 100 );
}

void Navigator::setSelected( QListBoxItem *i, bool sel )
{
  // Reimplemented to avoid the immediate activation of
  // the item. might turn out it doesn't work, we check that
  // an confirm from MainWindow::selectPlugin()
  if (sel) {
    EntryItem *entry = static_cast<EntryItem *>( i );
    emit pluginActivated( entry->plugin() );
  }
}

void Navigator::updatePlugins( QValueList<Kontact::Plugin*> plugins_ )
{
  QValueList<Kontact::PluginProxy> plugins;
  QValueList<Kontact::Plugin*>::ConstIterator end_ = plugins_.end();
  QValueList<Kontact::Plugin*>::ConstIterator it_ = plugins_.begin();
  for ( ; it_ != end_; ++it_ )
    plugins += PluginProxy( *it_ );

  clear();

  mActions.setAutoDelete( true );
  mActions.clear();
  mActions.setAutoDelete( false );

  int counter = 0;
  int minWidth = 0;
  qBubbleSort( plugins );
  QValueList<Kontact::PluginProxy>::ConstIterator end = plugins.end();
  QValueList<Kontact::PluginProxy>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = ( *it ).plugin();
    if ( !plugin->showInSideBar() )
      continue;

    EntryItem *item = new EntryItem( this, plugin );

    if ( item->width( this ) > minWidth )
      minWidth = item->width( this );

    QString name = QString( "CTRL+%1" ).arg( counter + 1 );
    KAction *action = new KAction( plugin->title(), KShortcut( name ),
                                   mMapper, SLOT( map() ),
                                   mSidePane->actionCollection(), name.latin1() );
    mMapper->setMapping( action, counter );
    counter++;
  }

  parentWidget()->setFixedWidth( minWidth );
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

void Navigator::slotExecuted( QListBoxItem *item )
{
  if ( !item ) return;
  
  EntryItem *entry = static_cast<EntryItem *>( item );

  emit pluginActivated( entry->plugin() );
}

IconViewMode Navigator::sizeIntToEnum(int size) const
{
  switch ( size ) { 
    case int(LargeIcons):
      return LargeIcons;
      break;
    case int(NormalIcons):
      return NormalIcons;
      break;
    case int(SmallIcons):
      return SmallIcons;
      break;
    case int(TextOnly):
      return TextOnly;
      break;
    default:
      // Stick with sane values
      return NormalIcons;
      kdDebug() << "View mode not implemented!" << endl;
      break;
  }
  
}

void Navigator::slotShowRMBMenu( QListBoxItem *, const QPoint& pos )
{
  KPopupMenu menu;
  menu.insertTitle( i18n("Icon Size") );
  menu.insertItem( i18n("Large"), (int)LargeIcons );
  menu.insertItem( i18n("Normal"), (int)NormalIcons );
  menu.insertItem( i18n("Small"), (int)SmallIcons );
  menu.insertItem( i18n("Text Only"), (int)TextOnly );
  int choice = menu.exec( pos );
  
  mViewMode = sizeIntToEnum(choice);
  Prefs::self()->setSidePaneIconSize( choice );

  triggerUpdate( true );
}

void Navigator::shortCutSelected( int pos )
{
  setCurrentItem( pos );
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
