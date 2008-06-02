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

#include "iconsidepane.h"
#include "mainwindow.h"
#include "plugin.h"
#include "prefs.h"

#include <kmenu.h>
#include <kdialog.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kicon.h>

#include <QApplication>
#include <q3ptrlist.h>
#include <q3signal.h>
#include <QObject>
#include <QFrame>
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QBitmap>
#include <QFontMetrics>
#include <QSignalMapper>
#include <QStyle>
#include <qdrawutil.h>
#include <QCursor>
#include <QTimer>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QStyleOptionViewItemV4>

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
  if ( size != 0 ) {
    mPixmap = KIconLoader::global()->loadIcon(
      mPlugin->icon(), KIconLoader::Desktop, size,
      mPlugin->disabled() ? KIconLoader::DisabledState : KIconLoader::DefaultState );
  } else {
    mPixmap = QPixmap();
  }
}

Navigator *EntryItem::navigator() const
{
  return static_cast<Navigator*>( listBox() );
}

int EntryItem::width( const Q3ListBox *listbox ) const
{
  int w = 0;
  if ( navigator()->showIcons() ) {
    w = navigator()->viewMode();
    if ( navigator()->viewMode() == SmallIcons ) {
      w += 4;
    }
  }
  if ( navigator()->showText() ) {
    if ( navigator()->viewMode() == SmallIcons ) {
      w += listbox->fontMetrics().width( text() );
    } else {
      w = qMax( w, listbox->fontMetrics().width( text() ) );
    }
  }
  return w + ( KDialog::marginHint() * 2 );
}

int EntryItem::height( const Q3ListBox *listbox ) const
{
  // the vertical margins consist of 2px outside the PE_PanelItemViewItem
  // to provide space between items and 2px inside the background
  const int verticalMargins = 8;
  int h = 0;
  if ( navigator()->showIcons() ) {
    h = (int)navigator()->viewMode() + verticalMargins;
  }
  if ( navigator()->showText() ) {
    if ( navigator()->viewMode() == SmallIcons || !navigator()->showIcons() ) {
      h = qMax( h, listbox->fontMetrics().lineSpacing() ) + KDialog::spacingHint() * 2;
    } else {
      h = (int)navigator()->viewMode() + listbox->fontMetrics().lineSpacing() + verticalMargins;
    }
  }
  return h;
}

void EntryItem::paint( QPainter *p )
{
  reloadPixmap();

  Q3ListBox *box = listBox();
  bool iconAboveText = ( navigator()->viewMode() > SmallIcons ) && navigator()->showIcons();
  int w = box->viewport()->width();
  int y = iconAboveText ? 4 : ( ( height( box ) - mPixmap.height() ) / 2 );
  int h = height( box );
  int spacing = KDialog::spacingHint();
  int pixmapSize = navigator()->viewMode();

  // draw background
  p->fillRect( QRect( 0, 0, w, h ), box->palette().background() );
  if ( isCurrent() || isSelected() || mHasHover || mPaintActive ) {
    QStyleOptionViewItemV4 o;
    o.initFrom( box );
    o.rect = QRect( 2, 1, w - 4, h - 3 );
    o.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;

    if ( isCurrent() || isSelected() ) {
      o.state |= QStyle::State_Selected;
    } else if ( mHasHover || mPaintActive ) {
      o.state |= QStyle::State_MouseOver;
    }

    box->style()->drawPrimitive( QStyle::PE_PanelItemViewItem, &o, p, box );
  }

  if ( !mPixmap.isNull() && navigator()->showIcons() ) {
    int x = iconAboveText ? ( ( w - mPixmap.width() ) / 2 ) :
                              spacing;
    p->drawPixmap( x, y, mPixmap );
  }

  if ( !text().isEmpty() && navigator()->showText() ) {
    QFontMetrics fm = p->fontMetrics();

    int x = 0;
    if ( iconAboveText ) {
      x = ( w - fm.width( text() ) ) / 2;
      y += fm.height() - fm.descent();
      if ( navigator()->showIcons() ) {
        y += mPixmap.height();
      }
    } else {
      x = spacing;
      if( navigator()->showIcons() ) {
        x += ( mPixmap.isNull() ? pixmapSize : mPixmap.width() ) + spacing;
      }

      if ( !navigator()->showIcons() || mPixmap.height() < fm.height() ) {
        y = height( box ) / 2 - fm.height() / 2 + fm.ascent();
      } else {
        y += mPixmap.height() / 2 - fm.height() / 2 + fm.ascent();
      }
    }

    if ( plugin()->disabled() ) {
      p->setPen( box->palette().disabled().text( ) );
    } else {
      p->setPen( box->palette().color( QPalette::Text ) );
    }
    p->drawText( x, y, text() );
  }

  // ensure that we don't have a stale flag around
  if ( isCurrent() || isSelected() ) {
    mHasHover = false;
  }
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
  : K3ListBox( parent, name ), mSidePane( parent ),
    mShowIcons( true ), mShowText( true )
{
  setFrameStyle( QFrame::NoFrame );
  mMouseOn = 0;
  mHighlightItem = 0;
  mViewMode = sizeIntToEnum( Prefs::self()->sidePaneIconSize() );
  mShowIcons = Prefs::self()->sidePaneShowIcons();
  mShowText = Prefs::self()->sidePaneShowText();
  setSelectionMode( K3ListBox::Single );
  viewport()->setBackgroundRole( QPalette::Background );
  setHScrollBarMode( Q3ScrollView::AlwaysOff );
  setAcceptDrops( true );

  setFocusPolicy( Qt::NoFocus );

  connect( this, SIGNAL(selectionChanged(Q3ListBoxItem *)),
           SLOT(slotExecuted(Q3ListBoxItem *)) );
  connect( this, SIGNAL(rightButtonPressed(Q3ListBoxItem *,const QPoint &)),
           SLOT(slotShowRMBMenu(Q3ListBoxItem *,const QPoint &)) );
  connect( this, SIGNAL(onItem(Q3ListBoxItem *)),
            SLOT(slotMouseOn(Q3ListBoxItem *)) );
  connect( this, SIGNAL(onViewport()), SLOT(slotMouseOff()) );

  mMapper = new QSignalMapper( this );
  connect( mMapper, SIGNAL(mapped(int)), SLOT(shortCutSelected(int)) );

  this->setToolTip( "" );
#ifdef __GNUC__
#warning Port me!
#endif
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

  QTimer::singleShot( 2000, this, SLOT(slotStopHighlight()) );
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

void Navigator::updatePlugins( QList<Kontact::Plugin*> plugins_ )
{
  QList<Kontact::PluginProxy> plugins;
  QList<Kontact::Plugin*>::ConstIterator end_ = plugins_.end();
  QList<Kontact::Plugin*>::ConstIterator it_ = plugins_.begin();
  for ( ; it_ != end_; ++it_ ) {
    plugins += PluginProxy( *it_ );
  }

  clear();

  qDeleteAll( mActions );
  mActions.clear();

  int counter = 0;
  int minWidth = 0;
#ifdef __GNUC__
#warning Port me!
#endif
//  qSort( plugins );
  QList<Kontact::PluginProxy>::ConstIterator end = plugins.end();
  QList<Kontact::PluginProxy>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Kontact::Plugin *plugin = ( *it ).plugin();
    if ( !plugin->showInSideBar() ) {
      continue;
    }

    EntryItem *item = new EntryItem( this, plugin );
    item->setSelectable( !plugin->disabled() );

    if ( item->width( this ) > minWidth ) {
      minWidth = item->width( this );
    }

    QString name = QString( "CTRL+%1" ).arg( counter + 1 );
    KAction *action = new KAction( KIcon( plugin->icon() ), plugin->title(), this );
    mSidePane->actionCollection()->addAction( name.toLatin1(), action );
    connect( action, SIGNAL(triggered(bool)), mMapper, SLOT(map()) );
    action->setShortcut( KShortcut( name ) );
    mActions.append( action );
    mMapper->setMapping( action, counter );
    counter++;
  }

  parentWidget()->setFixedWidth( minWidth );
}

void Navigator::dragEnterEvent( QDragEnterEvent *event )
{
  kDebug();

  dragMoveEvent( event );
}

void Navigator::dragMoveEvent( QDragMoveEvent *event )
{
  kDebug();

  kDebug() << "  Format:" << event->format();

  Q3ListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    event->setAccepted( false );
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kDebug() << "  PLUGIN:" << entry->plugin()->identifier();

  event->setAccepted( entry->plugin()->canDecodeMimeData( event->mimeData() ) );
}

void Navigator::dropEvent( QDropEvent *event )
{
  kDebug();

  kDebug() << "  Format:" << event->format();

  Q3ListBoxItem *item = itemAt( event->pos() );

  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  kDebug() << "  PLUGIN:" << entry->plugin()->identifier();

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
  K3ListBox::enterEvent( event );
  emit onItem( itemAt( mapFromGlobal( QCursor::pos() ) ) );
}

void Navigator::leaveEvent( QEvent *event )
{
  K3ListBox::leaveEvent( event );
  slotMouseOn( 0 );
  mMouseOn = 0;
}

void Navigator::slotExecuted( Q3ListBoxItem *item )
{
  if ( !item ) {
    return;
  }

  EntryItem *entry = static_cast<EntryItem*>( item );

  emit pluginActivated( entry->plugin() );
}

IconViewMode Navigator::sizeIntToEnum( int size ) const
{
  switch ( size ) {
  case int( LargeIcons ):
    return LargeIcons;
    break;
  case int( NormalIcons ):
    return NormalIcons;
    break;
  case int( SmallIcons ):
    return SmallIcons;
    break;
  default:
    // Stick with sane values
    return NormalIcons;
    kDebug() << "View mode not implemented!";
    break;
  }
}

void Navigator::slotShowRMBMenu( Q3ListBoxItem *, const QPoint &pos )
{
  KMenu menu( i18nc( "@title:menu", "Icon Size" ) );

  QAction *large = menu.addAction(
    i18nc( "@action:inmenu change to large icons", "Large" ) );
  large->setEnabled( mShowIcons );
  large->setCheckable( true );
  large->setChecked( mViewMode == LargeIcons );
  QAction *normal = menu.addAction(
    i18nc( "@action:inmenu change to normal size icons", "Normal" ) );
  normal->setEnabled( mShowIcons );
  normal->setCheckable( true );
  normal->setChecked( mViewMode == NormalIcons );
  QAction *small = menu.addAction(
    i18nc( "@action:inmenu change to small icons", "Small" ) );
  small->setEnabled( mShowIcons );
  small->setCheckable( true );
  small->setChecked( mViewMode == SmallIcons );

  menu.addSeparator();

  QAction *showIcons = menu.addAction(
    i18nc( "@action:inmenu show icons in sidepane", "Show Icons" ) );
  showIcons->setCheckable( true );
  showIcons->setChecked( mShowIcons );
  showIcons->setEnabled( mShowText );
  QAction *showText = menu.addAction(
    i18nc( "@action:inmenu show text under icons in sidepane", "Show Text" ) );
  showText->setCheckable( true );
  showText->setChecked( mShowText );
  showText->setEnabled( mShowIcons );

  QAction *choice = menu.exec( pos );

  if ( choice == 0 ) {
    return;
  }

  if ( choice == large ) {
    mViewMode = sizeIntToEnum( LargeIcons );
    Prefs::self()->setSidePaneIconSize( LargeIcons );
  } else if ( choice == normal ) {
    mViewMode = sizeIntToEnum( NormalIcons );
    Prefs::self()->setSidePaneIconSize( NormalIcons );
  } else if ( choice == small ) {
    mViewMode = sizeIntToEnum( SmallIcons );
    Prefs::self()->setSidePaneIconSize( SmallIcons );
  } else if ( choice == showIcons ) {
    mShowIcons = !mShowIcons;
    Prefs::self()->setSidePaneShowIcons( mShowIcons );
  } else if ( choice == showText ) {
    mShowText = !mShowText;
    Prefs::self()->setSidePaneShowText( mShowText );
  }

  int maxWidth = 0;
  Q3ListBoxItem *it = 0;
  for ( int i = 0; ( it = item(i) ) != 0; ++i ) {
    int width = it->width(this);
    if ( width > maxWidth ) {
      maxWidth = width;
    }
  }
  parentWidget()->setFixedWidth( maxWidth );

  triggerUpdate( true );
}

void Navigator::shortCutSelected( int pos )
{
  setCurrentItem( pos );
}

void Navigator::setHoverItem( Q3ListBoxItem *item, bool hover )
{
    static_cast<EntryItem*>( item )->setHover( hover );
    updateItem( item );
}

void Navigator::setPaintActiveItem( Q3ListBoxItem *item, bool paintActive )
{
    static_cast<EntryItem*>( item )->setPaintActive( paintActive );
    updateItem( item );
}

void Navigator::slotMouseOn( Q3ListBoxItem *newItem )
{
  Q3ListBoxItem *oldItem = mMouseOn;
  if ( oldItem == newItem ) {
    return;
  }

  if ( oldItem && !oldItem->isCurrent() && !oldItem->isSelected() ) {
    setHoverItem( oldItem, false );
  }

  if ( newItem && !newItem->isCurrent() && !newItem->isSelected() ) {
    setHoverItem( newItem, true );
  }
  mMouseOn = newItem;
}

void Navigator::slotMouseOff()
{
  slotMouseOn( 0 );
}

IconSidePane::IconSidePane( Core *core, QWidget *parent )
  : SidePaneBase( core, parent )
{
  mNavigator = new Navigator( this );
  connect( mNavigator, SIGNAL(pluginActivated(Kontact::Plugin *)),
           SIGNAL(pluginSelected(Kontact::Plugin *)) );

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
