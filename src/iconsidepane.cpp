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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include <q3ptrlist.h>
#include <q3widgetstack.h>
#include <q3signal.h>
#include <qobject.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qfontmetrics.h>
#include <qsignalmapper.h>
#include <qstyle.h>
#include <q3frame.h>
#include <qdrawutil.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QPixmap>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>
#include <Q3ValueList>
#include <QResizeEvent>
#include <QDragEnterEvent>

#include <kpopupmenu.h>
#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>

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
      : mPlugin( 0 )
    { }

    PluginProxy( Plugin *plugin )
      : mPlugin( plugin )
    { }

    PluginProxy & operator=( Plugin *plugin )
    {
      mPlugin = plugin;
      return *this;
    }

    bool operator<( PluginProxy &rhs ) const
    {
      return mPlugin->weight() < rhs.mPlugin->weight();
    }

    Plugin *plugin() const
    {
      return mPlugin;
    }

  private:
    Plugin *mPlugin;
};

} //namespace

using namespace Kontact;

EntryItem::EntryItem( Navigator *parent, Kontact::Plugin *plugin )
  : Q3ListBoxItem( parent ),
    mPlugin( plugin ),
    mHasHover( false ),
    mPaintActive( false )
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
  if ( size != 0 )
    mPixmap = KGlobal::iconLoader()->loadIcon( mPlugin->icon(),
                                               KIcon::Desktop, size );
  else
    mPixmap = QPixmap();
}

Navigator* EntryItem::navigator() const
{
  return static_cast<Navigator*>( listBox() );
}

int EntryItem::width( const Q3ListBox *listbox ) const
{
  int w = 0;
  if( navigator()->showIcons() ) {
    w = navigator()->viewMode();
    if ( navigator()->viewMode() == SmallIcons )
      w += 4;
  }
  if( navigator()->showText() ) {
    if ( navigator()->viewMode() == SmallIcons )
      w += listbox->fontMetrics().width( text() );
    else
      w = qMax( w, listbox->fontMetrics().width( text() ) );
  }
  return w + ( KDialog::marginHint() * 2 );
}

int EntryItem::height( const Q3ListBox *listbox ) const
{
  int h = 0;
  if ( navigator()->showIcons() )
    h = (int)navigator()->viewMode() + 4;
  if ( navigator()->showText() ) {
    if ( navigator()->viewMode() == SmallIcons || !navigator()->showIcons() )
      h = qMax( h, listbox->fontMetrics().lineSpacing() ) + KDialog::spacingHint() * 2;
    else
      h = (int)navigator()->viewMode() + listbox->fontMetrics().lineSpacing() + 4;
  }
  return h;
}

void EntryItem::paint( QPainter *p )
{
  reloadPixmap();

  Q3ListBox *box = listBox();
  bool iconAboveText = ( navigator()->viewMode() > SmallIcons )
                     && navigator()->showIcons();
  int w = box->viewport()->width();
  int y = iconAboveText ? 2 :
                        ( ( height( box ) - mPixmap.height() ) / 2 );

  // draw selected
  if ( isCurrent() || isSelected() || mHasHover || mPaintActive ) {
    int h = height( box );

    QBrush brush;
    if ( isCurrent() || isSelected() || mPaintActive )
      brush = box->colorGroup().brush( QColorGroup::Highlight );
    else
      brush = box->colorGroup().highlight().light( 115 );
    p->fillRect( 1, 0, w - 2, h - 1, brush );
    QPen pen = p->pen();
    QPen oldPen = pen;
    pen.setColor( box->colorGroup().mid() );
    p->setPen( pen );

    p->drawPoint( 1, 0 );
    p->drawPoint( 1, h - 2 );
    p->drawPoint( w - 2, 0 );
    p->drawPoint( w - 2, h - 2 );

    p->setPen( oldPen );
  }

  if ( !mPixmap.isNull() && navigator()->showIcons() ) {
      int x = iconAboveText ? ( ( w - mPixmap.width() ) / 2 ) :
                              KDialog::marginHint();
    p->drawPixmap( x, y, mPixmap );
  }

  QColor shadowColor = listBox()->colorGroup().background().dark(115);
  if ( isCurrent() || isSelected() ) {
    p->setPen( box->colorGroup().highlightedText() );
  }

  if ( !text().isEmpty() && navigator()->showText() ) {
    QFontMetrics fm = p->fontMetrics();

    int x = 0;
    if ( iconAboveText ) {
      x = ( w - fm.width( text() ) ) / 2;
      y += fm.height() - fm.descent();
      if ( navigator()->showIcons() )
        y += mPixmap.height();
    } else {
      x = KDialog::marginHint() + 4;
      if( navigator()->showIcons() ) {
        x += mPixmap.width();
      }

      if ( !navigator()->showIcons() || mPixmap.height() < fm.height() )
        y = height( box )/2 - fm.height()/2 + fm.ascent();
      else
        y += mPixmap.height()/2 - fm.height()/2 + fm.ascent();
    }

    if ( isCurrent() || isSelected() || mHasHover ) {
      p->setPen( box->colorGroup().highlight().dark(115) );
      p->drawText( x + ( QApplication::reverseLayout() ? -1 : 1),
                   y + 1, text() );
      p->setPen( box->colorGroup().highlightedText() );
    }
    else
      p->setPen( box->colorGroup().text() );

    p->drawText( x, y, text() );
  }

  // ensure that we don't have a stale flag around
  if (  isCurrent() || isSelected() ) mHasHover = false;
}

void EntryItem::setHover( bool hasHover )
{
  mHasHover = hasHover;
}

void EntryItem::setPaintActive( bool paintActive )
{
  mPaintActive = paintActive;
}

Navigator::Navigator( SidePaneBase *parent, const char *name )
  : KListBox( parent, name ), mSidePane( parent ),
    mShowIcons( true ), mShowText( true )
{
  mMouseOn = 0;
  mHighlightItem = 0;
  mViewMode = sizeIntToEnum( Prefs::self()->sidePaneIconSize() );
  mShowIcons = Prefs::self()->sidePaneShowIcons();
  mShowText = Prefs::self()->sidePaneShowText();
  setSelectionMode( KListBox::Single );
  viewport()->setBackgroundMode( Qt::PaletteBackground );
  setFrameStyle( Q3Frame::NoFrame );
  setHScrollBarMode( Q3ScrollView::AlwaysOff );
  setAcceptDrops( true );

  setFocusPolicy( Qt::NoFocus );

  connect( this, SIGNAL( selectionChanged( Q3ListBoxItem* ) ),
           SLOT( slotExecuted( Q3ListBoxItem* ) ) );
  connect( this, SIGNAL( rightButtonPressed( Q3ListBoxItem*, const QPoint& ) ),
           SLOT( slotShowRMBMenu( Q3ListBoxItem*, const QPoint& ) ) );
  connect( this, SIGNAL( onItem( Q3ListBoxItem * ) ),
            SLOT(  slotMouseOn( Q3ListBoxItem * ) ) );
  connect( this, SIGNAL( onViewport() ), SLOT(  slotMouseOff() ) );

  mMapper = new QSignalMapper( this );
  connect( mMapper, SIGNAL( mapped( int ) ), SLOT( shortCutSelected( int ) ) );

  QToolTip::remove( this );
#warning Port me!
//  if ( !mShowText )
//    new EntryItemToolTip( this );

}

QSize Navigator::sizeHint() const
{
  return QSize( 100, 100 );
}

void Navigator::highlightItem( EntryItem * item )
{
  mHighlightItem = item;

  setPaintActiveItem( mHighlightItem, true );

  QTimer::singleShot( 2000, this, SLOT( slotStopHighlight() ) );
}

void Navigator::slotStopHighlight()
{
  setPaintActiveItem( mHighlightItem, false );
}

void Navigator::setSelected( Q3ListBoxItem *item, bool selected )
{
  // Reimplemented to avoid the immediate activation of
  // the item. might turn out it doesn't work, we check that
  // an confirm from MainWindow::selectPlugin()
  if ( selected ) {
    EntryItem *entry = static_cast<EntryItem*>( item );
    emit pluginActivated( entry->plugin() );
  }
}

void Navigator::updatePlugins( Q3ValueList<Kontact::Plugin*> plugins_ )
{
  QList<Kontact::PluginProxy> plugins;
  Q3ValueList<Kontact::Plugin*>::ConstIterator end_ = plugins_.end();
  Q3ValueList<Kontact::Plugin*>::ConstIterator it_ = plugins_.begin();
  for ( ; it_ != end_; ++it_ )
    plugins += PluginProxy( *it_ );

  clear();

  mActions.setAutoDelete( true );
  mActions.clear();
  mActions.setAutoDelete( false );

  int counter = 0;
  int minWidth = 0;
#warning Port me!
//  qSort( plugins );
  QList<Kontact::PluginProxy>::ConstIterator end = plugins.end();
  QList<Kontact::PluginProxy>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = ( *it ).plugin();
    if ( !plugin->showInSideBar() )
      continue;

    EntryItem *item = new EntryItem( this, plugin );

    if ( item->width( this ) > minWidth )
      minWidth = item->width( this );

    QString name = QString( "CTRL+%1" ).arg( counter + 1 );
    KAction *action = new KAction( plugin->title(), plugin->icon(), KShortcut( name ),
                                   mMapper, SLOT( map() ),
                                   mSidePane->actionCollection(), name.latin1() );
    mActions.append( action );
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

  Q3ListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    event->accept( false );
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kdDebug(5600) << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  event->accept( entry->plugin()->canDecodeDrag( event ) );
}

void Navigator::dropEvent( QDropEvent *event )
{
  kdDebug(5600) << "Navigator::dropEvent()" << endl;

  Q3ListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kdDebug(5600) << "  PLUGIN: " << entry->plugin()->identifier() << endl;

  entry->plugin()->processDropEvent( event );
}

void Navigator::resizeEvent( QResizeEvent *event )
{
  Q3ListBox::resizeEvent( event );
  triggerUpdate( true );
}

void Navigator::enterEvent( QEvent *event )
{
  // work around Qt behaviour: onItem is not emmitted in enterEvent()
  KListBox::enterEvent( event );
  emit onItem( itemAt( mapFromGlobal( QCursor::pos() ) ) );
}

void Navigator::leaveEvent( QEvent *event )
{
  KListBox::leaveEvent( event );
  slotMouseOn( 0 );
  mMouseOn = 0;
}

void Navigator::slotExecuted( Q3ListBoxItem *item )
{
  if ( !item )
    return;

  EntryItem *entry = static_cast<EntryItem*>( item );

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
    default:
      // Stick with sane values
      return NormalIcons;
      kdDebug() << "View mode not implemented!" << endl;
      break;
  }
}

void Navigator::slotShowRMBMenu( Q3ListBoxItem *, const QPoint &pos )
{
  KPopupMenu menu;
  menu.insertTitle( i18n( "Icon Size" ) );
  menu.insertItem( i18n( "Large" ), (int)LargeIcons );
  menu.setItemEnabled( (int)LargeIcons, mShowIcons );
  menu.insertItem( i18n( "Normal" ), (int)NormalIcons );
  menu.setItemEnabled( (int)NormalIcons, mShowIcons );
  menu.insertItem( i18n( "Small" ), (int)SmallIcons );
  menu.setItemEnabled( (int)SmallIcons, mShowIcons );

  menu.setItemChecked( (int)mViewMode, true );
  menu.insertSeparator();

  menu.insertItem( i18n( "Show Icons" ), (int)ShowIcons );
  menu.setItemChecked( (int)ShowIcons, mShowIcons );
  menu.setItemEnabled( (int)ShowIcons, mShowText );
  menu.insertItem( i18n( "Show Text" ), (int)ShowText );
  menu.setItemChecked( (int)ShowText, mShowText );
  menu.setItemEnabled( (int)ShowText, mShowIcons );
  int choice = menu.exec( pos );

  if ( choice == -1 )
    return;

  if ( choice >= SmallIcons ) {
    mViewMode = sizeIntToEnum( choice );
    Prefs::self()->setSidePaneIconSize( choice );
  } else {
    // either icons or text were toggled
    if ( choice == ShowIcons ) {
      mShowIcons = !mShowIcons;
      Prefs::self()->setSidePaneShowIcons( mShowIcons );
      QToolTip::remove( this );
#warning Port me!
//      if ( !mShowText )
//        new EntryItemToolTip( this );
    } else {
      mShowText = !mShowText;
      Prefs::self()->setSidePaneShowText( mShowText );
      QToolTip::remove( this );
    }
  }
  int maxWidth = 0;
  Q3ListBoxItem* it = 0;
  for (int i = 0; (it = item(i)) != 0; ++i)
  {
    int width = it->width(this);
    if (width > maxWidth)
      maxWidth = width;
  }
  parentWidget()->setFixedWidth( maxWidth );

  triggerUpdate( true );
}

void Navigator::shortCutSelected( int pos )
{
  setCurrentItem( pos );
}

void Navigator::setHoverItem( Q3ListBoxItem* item, bool hover )
{
    static_cast<EntryItem*>( item )->setHover( hover );
    updateItem( item );
}

void Navigator::setPaintActiveItem( Q3ListBoxItem* item, bool paintActive )
{
    static_cast<EntryItem*>( item )->setPaintActive( paintActive );
    updateItem( item );
}

void Navigator::slotMouseOn( Q3ListBoxItem* newItem )
{
  Q3ListBoxItem* oldItem = mMouseOn;
  if ( oldItem == newItem ) return;

  if ( oldItem && !oldItem->isCurrent() && !oldItem->isSelected() )
  {
    setHoverItem( oldItem, false );
  }

  if ( newItem && !newItem->isCurrent() && !newItem->isSelected() )
  {
    setHoverItem( newItem, true );
  }
  mMouseOn = newItem;
}

void Navigator::slotMouseOff()
{
  slotMouseOn( 0 );
}

IconSidePane::IconSidePane( Core *core, QWidget *parent, const char *name )
  : SidePaneBase( core, parent, name )
{
  mNavigator = new Navigator( this );
  connect( mNavigator, SIGNAL( pluginActivated( Kontact::Plugin* ) ),
           SIGNAL( pluginSelected( Kontact::Plugin* ) ) );

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

  for ( uint i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem*>( mNavigator->item( i ) );
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

  for ( uint i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem*>( mNavigator->item( i ) );
    if ( item->plugin()->identifier() == name ) {
      mNavigator->setCurrentItem( i );
      break;
    }
  }

  blockSignals( blocked );
}

void IconSidePane::indicateForegrunding( Kontact::Plugin *plugin )
{
  for ( uint i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem*>( mNavigator->item( i ) );
    if ( item->plugin() == plugin ) {
      mNavigator->highlightItem( item );
      break;
    }
  }


}
#include "iconsidepane.moc"

// vim: sw=2 sts=2 et tw=80
