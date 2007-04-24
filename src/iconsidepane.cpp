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

#include <qptrlist.h>
#include <qwidgetstack.h>
#include <qsignal.h>
#include <qobjectlist.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qfontmetrics.h>
#include <qsignalmapper.h>
#include <qstyle.h>
#include <qframe.h>
#include <qdrawutil.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <kpopupmenu.h>
#include <kpixmapeffect.h>
#include <kplugininfo.h>
#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "plugin.h"

#include "prefs.h"
#include "iconsidepane.h"

namespace Kontact {

// wrapper class to uniformly handle loaded plugins and disabled plugins where we
// only have the plugin info available, not the plugin itself

class PluginProxy
{
  public:
    PluginProxy()
      : mPlugin( 0 ), mWeight( 0 ), mShowInSideBar( false )
    { }

    explicit PluginProxy( Plugin *plugin )
      : mPlugin( plugin ), mIcon( plugin->icon() ), mTitle( plugin->title() ), mIdentifier( plugin->identifier() ), mWeight( plugin->weight() ), mShowInSideBar( plugin->showInSideBar() )
    {
    }

    explicit PluginProxy( KPluginInfo* info ) : mPlugin( 0 ), mIcon( info->icon() ),  mTitle( info->name() ), mIdentifier( info->pluginName() ), mWeight( 1000 ), mShowInSideBar( true )
    {
      const QVariant hasPart = info->property( "X-KDE-KontactPluginHasPart" );
      if ( hasPart.isValid() )
        mShowInSideBar = hasPart.toBool();
    }

    Plugin* plugin() const {
      return mPlugin;
    }

    bool isPluginEnabled() const {
      return mPlugin != 0L;
    }

    bool operator<( PluginProxy &rhs ) const {
      return weight() < rhs.weight();
    }

    QString icon() const {
      return mIcon;
    }

    QString title() const {
      return mTitle;
    }

    void processDropEvent( QDropEvent* event ) {
      if ( mPlugin )
        processDropEvent( event );
    }

    bool canDecodeDrag( QMimeSource * source ) const {
      return mPlugin ? mPlugin->canDecodeDrag( source ) : false;
    }
    QString identifier() const {
      return mIdentifier;
    }

    int weight() const {
      return mWeight;
    }

    bool showInSideBar() const {
      return mShowInSideBar;
    }

  private:

    Plugin *mPlugin;
    QString mIcon;
    QString mTitle;
    QString mIdentifier;
    int mWeight;
    bool mShowInSideBar;
};

/**
  A QListBoxPixmap Square Box with an optional icon and a text
  underneath.
*/
class EntryItem : public QListBoxItem
{
  public:
    EntryItem( Navigator *, const PluginProxy& );
    ~EntryItem();

    Kontact::PluginProxy pluginProxy() const { return mPluginProxy; }

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
// 
    virtual void paint( QPainter *p );

  private:
    Kontact::PluginProxy mPluginProxy;
    QPixmap mPixmap;

    bool mHasHover;
    bool mPaintActive;
};

/**
 * Tooltip that changes text depending on the item it is above.
 * Compliments of "Practical Qt" by Dalheimer, Petersen et al.
 */
class EntryItemToolTip : public QToolTip
{
  public:
    EntryItemToolTip( QListBox* parent )
      : QToolTip( parent->viewport() ), mListBox( parent )
      {}
  protected:
    void maybeTip( const QPoint& p ) {
      // We only show tooltips when there are no texts shown
      if ( Prefs::self()->sidePaneShowText() ) return;
      if ( !mListBox ) return;
      QListBoxItem* item = mListBox->itemAt( p );
      if ( !item ) return;
      const QRect itemRect = mListBox->itemRect( item );
      if ( !itemRect.isValid() ) return;

      const EntryItem *entryItem = static_cast<EntryItem*>( item );
      QString tipStr = entryItem->text();
      tip( itemRect, tipStr );
    }
  private:
    QListBox* mListBox;
};

} // namespace Kontact

using namespace Kontact;

EntryItem::EntryItem( Navigator *parent, const PluginProxy &proxy )
  : QListBoxItem( parent ),
    mPluginProxy( proxy ),
    mHasHover( false ),
    mPaintActive( false )
{
  reloadPixmap();
  setCustomHighlighting( true );
  setText( proxy.title() );
}

EntryItem::~EntryItem()
{
}

void EntryItem::reloadPixmap()
{
  int size = (int)navigator()->viewMode();
  if ( size != 0 )
    mPixmap = KGlobal::iconLoader()->loadIcon( mPluginProxy.icon(),
                                               KIcon::Desktop, size, pluginProxy().isPluginEnabled() ? KIcon::DefaultState : KIcon::DisabledState );
  else
    mPixmap = QPixmap();
}

Navigator* EntryItem::navigator() const
{
  return static_cast<Navigator*>( listBox() );
}

int EntryItem::width( const QListBox *listbox ) const
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
      w = QMAX( w, listbox->fontMetrics().width( text() ) );
  }
  return w + ( KDialog::marginHint() * 2 );
}

int EntryItem::height( const QListBox *listbox ) const
{
  int h = 0;
  if ( navigator()->showIcons() )
    h = (int)navigator()->viewMode() + 4;
  if ( navigator()->showText() ) {
    if ( navigator()->viewMode() == SmallIcons || !navigator()->showIcons() )
      h = QMAX( h, listbox->fontMetrics().lineSpacing() ) + KDialog::spacingHint() * 2;
    else
      h = (int)navigator()->viewMode() + listbox->fontMetrics().lineSpacing() + 4;
  }
  return h;
}

void EntryItem::paint( QPainter *p )
{
  reloadPixmap();

  QListBox *box = listBox();
  bool iconAboveText = ( navigator()->viewMode() > SmallIcons )
                     && navigator()->showIcons();
  int w = box->viewport()->width();
  int y = iconAboveText ? 2 :
                        ( ( height( box ) - mPixmap.height() ) / 2 );

  const bool isEnabled = pluginProxy().isPluginEnabled();
  // draw selected
  if ( isEnabled && ( isCurrent() || isSelected() || mHasHover || mPaintActive ) )  {
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

    if ( !pluginProxy().isPluginEnabled() ) {
        p->setPen( qApp->palette().disabled().text() );
    }
    else if ( isCurrent() || isSelected() || mHasHover ) {
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
  viewport()->setBackgroundMode( PaletteBackground );
  setFrameStyle( QFrame::NoFrame );
  setHScrollBarMode( QScrollView::AlwaysOff );
  setAcceptDrops( true );

  setFocusPolicy( NoFocus );

  connect( this, SIGNAL( selectionChanged( QListBoxItem* ) ),
           SLOT( slotExecuted( QListBoxItem* ) ) );
  connect( this, SIGNAL( rightButtonPressed( QListBoxItem*, const QPoint& ) ),
           SLOT( slotShowRMBMenu( QListBoxItem*, const QPoint& ) ) );
  connect( this, SIGNAL( onItem( QListBoxItem * ) ),
            SLOT(  slotMouseOn( QListBoxItem * ) ) );
  connect( this, SIGNAL( onViewport() ), SLOT(  slotMouseOff() ) );

  mMapper = new QSignalMapper( this );
  connect( mMapper, SIGNAL( mapped( int ) ), SLOT( shortCutSelected( int ) ) );

  QToolTip::remove( this );
  if ( !mShowText )
    new EntryItemToolTip( this );

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

void Navigator::setSelected( QListBoxItem *item, bool selected )
{
  // Reimplemented to avoid the immediate activation of
  // the item. might turn out it doesn't work, we check that
  // an confirm from MainWindow::selectPlugin()
  if ( selected ) {
    EntryItem *entry = static_cast<EntryItem*>( item );
    emit pluginActivated( entry->pluginProxy().plugin() );
  }
}

void Navigator::updatePlugins(  const QValueList<Kontact::Plugin*> &plugins_,
                                const QValueList<KPluginInfo*> &disabled_ )
{
  QValueList<Kontact::PluginProxy> plugins;
  QValueList<Kontact::Plugin*>::ConstIterator end_ = plugins_.end();
  QValueList<Kontact::Plugin*>::ConstIterator it_ = plugins_.begin();
  for ( ; it_ != end_; ++it_ )
    plugins += PluginProxy( *it_ );

  QValueList<KPluginInfo*>::ConstIterator end2_ = disabled_.end();
  QValueList<KPluginInfo*>::ConstIterator it2_ = disabled_.begin();

  for ( ; it2_ != end2_; ++it2_ )
    plugins += PluginProxy( *it2_ );

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
    if ( !(*it).showInSideBar() )
      continue;

    EntryItem *item = new EntryItem( this, *it );
    item->setSelectable( (*it).isPluginEnabled() );

    if ( item->width( this ) > minWidth )
      minWidth = item->width( this );

    QString name = QString( "CTRL+%1" ).arg( counter + 1 );
    KAction *action = new KAction( (*it).title(), (*it).icon(), KShortcut( name ),
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

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    event->accept( false );
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kdDebug(5600) << "  PLUGIN: " << entry->pluginProxy().identifier() << endl;

  event->accept( entry->pluginProxy().canDecodeDrag( event ) );
}

void Navigator::dropEvent( QDropEvent *event )
{
  kdDebug(5600) << "Navigator::dropEvent()" << endl;

  QListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kdDebug(5600) << "  PLUGIN: " << entry->pluginProxy().identifier() << endl;

  entry->pluginProxy().processDropEvent( event );
}

void Navigator::resizeEvent( QResizeEvent *event )
{
  QListBox::resizeEvent( event );
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

void Navigator::slotExecuted( QListBoxItem *item )
{
  if ( !item )
    return;

  EntryItem *entry = static_cast<EntryItem*>( item );

  emit pluginActivated( entry->pluginProxy().plugin() );
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

void Navigator::slotShowRMBMenu( QListBoxItem *, const QPoint &pos )
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
      if ( !mShowText )
        new EntryItemToolTip( this );
    } else {
      mShowText = !mShowText;
      Prefs::self()->setSidePaneShowText( mShowText );
      QToolTip::remove( this );
    }
  }
  int maxWidth = 0;
  QListBoxItem* it = 0;
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

void Navigator::setHoverItem( QListBoxItem* item, bool hover )
{
    static_cast<EntryItem*>( item )->setHover( hover );
    updateItem( item );
}

void Navigator::setPaintActiveItem( QListBoxItem* item, bool paintActive )
{
    static_cast<EntryItem*>( item )->setPaintActive( paintActive );
    updateItem( item );
}

void Navigator::slotMouseOn( QListBoxItem* newItem )
{
  QListBoxItem* oldItem = mMouseOn;
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

void IconSidePane::updatePlugins( const QValueList<Kontact::Plugin*> &plugins,
                                  const QValueList<KPluginInfo*> &disabled )
{
  mNavigator->updatePlugins( plugins, disabled );
}

void IconSidePane::selectPlugin( Kontact::Plugin *plugin )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  for ( uint i = 0; i < mNavigator->count(); ++i ) {
    EntryItem *item = static_cast<EntryItem*>( mNavigator->item( i ) );
    if ( item->pluginProxy().plugin() == plugin ) {
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
    if ( item->pluginProxy().identifier() == name ) {
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
    if ( item->pluginProxy().plugin() == plugin ) {
      mNavigator->highlightItem( item );
      break;
    }
  }


}
#include "iconsidepane.moc"

// vim: sw=2 sts=2 et tw=80
