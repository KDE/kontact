/*
  This file is part of KDE Kontact.

  Copyright (C) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>

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

#include <QtGui/QStringListModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QScrollBar>

#include <KLocalizedString>
#include <KDialog>
#include <KIcon>

namespace Kontact
{

class Navigator;

class SelectionModel : public QItemSelectionModel
{
  public:
    SelectionModel( QAbstractItemModel *model, QObject *parent )
      : QItemSelectionModel( model, parent )
    {
    }

  public slots:
    virtual void clear()
    {
      // Don't allow the current selection to be cleared. QListView doesn't call to this method
      // nowadays, but just to cover of future change of implementation, since QTreeView does call
      // to this one when clearing the selection.
    }

    virtual void select( const QModelIndex &index, QItemSelectionModel::SelectionFlags command )
    {
      // Don't allow the current selection to be cleared
      if ( !index.isValid() && ( command & QItemSelectionModel::Clear ) ) {
        return;
      }
      QItemSelectionModel::select( index, command );
    }

    virtual void select( const QItemSelection &selection,
                         QItemSelectionModel::SelectionFlags command )
    {
      // Don't allow the current selection to be cleared
      if ( !selection.count() && ( command & QItemSelectionModel::Clear ) ) {
        return;
      }
      QItemSelectionModel::select( selection, command );
    }
};

class Model : public QStringListModel
{
  public:
    enum SpecialRoles {
      PluginName = Qt::UserRole
    };

    Model( Navigator *parentNavigator = 0 )
      : QStringListModel( parentNavigator ), mNavigator(parentNavigator)
    {
    }

    void emitLayoutChanged()
    {
        emit layoutChanged();
    }

    void setPluginList( const QList<Kontact::Plugin*> &list ) {
      pluginList = list;
    }

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const
    {
      Qt::ItemFlags flags = QStringListModel::flags( index );

      flags &= ~Qt::ItemIsEditable;

      if ( index.isValid() ) {
        if ( static_cast<Kontact::Plugin*>( index.internalPointer() )->disabled() ) {
          flags &= ~Qt::ItemIsEnabled;
          flags &= ~Qt::ItemIsSelectable;
          flags &= ~Qt::ItemIsDropEnabled;
        } else {
          flags |= Qt::ItemIsDropEnabled;
        }
      } else {
        flags &= ~Qt::ItemIsDropEnabled;
      }

      return flags;
    }

    virtual QModelIndex index( int row, int column,
                               const QModelIndex &parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );
      if ( row < 0 || row >= pluginList.count() ) {
        return QModelIndex();
      }
      return createIndex( row, column, pluginList[row] );
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( !index.isValid() || !index.internalPointer() ) {
        return QVariant();
      }

      if ( role == Qt::DisplayRole ) {
        if ( !mNavigator->showText() ) {
          return QVariant();
        }
        return static_cast<Kontact::Plugin*>( index.internalPointer() )->title();
      } else if ( role == Qt::DecorationRole ) {
        if ( !mNavigator->showIcons() ) {
          return QVariant();
        }
        return KIcon( static_cast<Kontact::Plugin*>( index.internalPointer() )->icon() );
      } else if ( role == Qt::TextAlignmentRole ) {
        return Qt::AlignCenter;
      } else if ( role == Qt::ToolTipRole ) {
        if ( !mNavigator->showText() ) {
          return static_cast<Kontact::Plugin*>( index.internalPointer() )->title();
        }
        return QVariant();
      } else if ( role == PluginName ) {
        return static_cast<Kontact::Plugin*>( index.internalPointer() )->identifier();
      }
      return QStringListModel::data( index, role );
    }

  private:
    QList<Kontact::Plugin*> pluginList;
    Navigator *mNavigator;
};

class SortFilterProxyModel
  : public QSortFilterProxyModel
{
  public:
    SortFilterProxyModel( QObject *parent = 0 ): QSortFilterProxyModel( parent )
    {
      setDynamicSortFilter( true );
      sort ( 0 );
    }
  protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const
    {
      Kontact::Plugin *leftPlugin = static_cast<Kontact::Plugin*>( left.internalPointer() );
      Kontact::Plugin *rightPlugin = static_cast<Kontact::Plugin*>( right.internalPointer() );
      return leftPlugin->weight() < rightPlugin->weight();
    }
};

class Delegate : public QStyledItemDelegate
{
  public:
    Delegate( Navigator *parentNavigator = 0 )
      : QStyledItemDelegate( parentNavigator ), mNavigator( parentNavigator )
    {
    }

    void paint( QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index ) const
    {
      if ( !index.isValid() || !index.internalPointer() ) {
        return;
      }

      QStyleOptionViewItemV4 optionCopy( *static_cast<const QStyleOptionViewItemV4*>( &option ) );
      optionCopy.decorationPosition = QStyleOptionViewItem::Top;
      optionCopy.decorationSize = QSize( mNavigator->iconSize(), mNavigator->iconSize() );
      optionCopy.textElideMode = Qt::ElideNone;
      QStyledItemDelegate::paint( painter, optionCopy, index );
    }

    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      if ( !index.isValid() || !index.internalPointer() ) {
        return QSize();
      }

      QStyleOptionViewItemV4 optionCopy( *static_cast<const QStyleOptionViewItemV4*>( &option ) );
      optionCopy.decorationPosition = QStyleOptionViewItem::Top;
      optionCopy.decorationSize =
        mNavigator->showIcons() ? QSize( mNavigator->iconSize(), mNavigator->iconSize() ) : QSize();
      optionCopy.textElideMode = Qt::ElideNone;
      return QStyledItemDelegate::sizeHint( optionCopy, index );
    }

  private:
    Navigator *mNavigator;
};

Navigator::Navigator( SidePaneBase *parent )
  : QListView( parent ), mSidePane( parent )
{
  setViewport( new QWidget( this ) );

  setVerticalScrollMode( ScrollPerPixel );
  setHorizontalScrollMode( ScrollPerPixel );

  mIconSize = Prefs::self()->sidePaneIconSize();
  mShowIcons = Prefs::self()->sidePaneShowIcons();
  mShowText = Prefs::self()->sidePaneShowText();

  QActionGroup *viewMode = new QActionGroup( this );

  mShowIconsAction = new KAction( i18n( "Show Icons Only" ), this );
  mShowIconsAction->setCheckable( true );
  mShowIconsAction->setActionGroup( viewMode );
  mShowIconsAction->setChecked( !mShowText && mShowIcons );
  connect( mShowIconsAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  mShowTextAction = new KAction( i18n( "Show Text Only" ), this );
  mShowTextAction->setCheckable( true );
  mShowTextAction->setActionGroup( viewMode );
  mShowTextAction->setChecked( mShowText && !mShowIcons );
  connect( mShowTextAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  mShowBothAction = new KAction( i18n( "Show Icons and Text" ), this );
  mShowBothAction->setCheckable( true );
  mShowBothAction->setActionGroup( viewMode );
  mShowBothAction->setChecked( mShowText && mShowIcons );
  connect( mShowBothAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  KAction *sep = new KAction( this );
  sep->setSeparator( true );

  QActionGroup *iconSize = new QActionGroup( this );

  mBigIconsAction = new KAction( i18n( "Big Icons" ), this );
  mBigIconsAction->setCheckable( iconSize );
  mBigIconsAction->setActionGroup( iconSize );
  mBigIconsAction->setChecked( mIconSize == KIconLoader::SizeLarge );
  connect( mBigIconsAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  mNormalIconsAction = new KAction( i18n( "Normal Icons" ), this );
  mNormalIconsAction->setCheckable( true );
  mNormalIconsAction->setActionGroup( iconSize );
  mNormalIconsAction->setChecked( mIconSize == KIconLoader::SizeMedium );
  connect( mNormalIconsAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  mSmallIconsAction = new KAction( i18n( "Small Icons" ), this );
  mSmallIconsAction->setCheckable( true );
  mSmallIconsAction->setActionGroup( iconSize );
  mSmallIconsAction->setChecked( mIconSize == KIconLoader::SizeSmallMedium );
  connect( mSmallIconsAction, SIGNAL(triggered(bool)), this, SLOT(slotActionTriggered(bool)) );

  QList<QAction*> actionList;
  actionList << mShowIconsAction << mShowTextAction << mShowBothAction << sep
             << mBigIconsAction << mNormalIconsAction << mSmallIconsAction;

  insertActions( 0, actionList );

  setContextMenuPolicy( Qt::ActionsContextMenu );
  setViewMode( ListMode );
  setItemDelegate( new Delegate( this ) );
  mModel = new Model( this );
  SortFilterProxyModel *sortFilterProxyModel = new SortFilterProxyModel;
  sortFilterProxyModel->setSourceModel( mModel );
  setModel( sortFilterProxyModel );
  setSelectionModel( new SelectionModel( sortFilterProxyModel, this ) );

  setDragDropMode( DropOnly );
  viewport()->setAcceptDrops( true );
  setDropIndicatorShown( true );

  connect( selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
           this, SLOT(slotCurrentChanged(QModelIndex)) );
}

void Navigator::updatePlugins( QList<Kontact::Plugin*> plugins_ )
{
  QString currentPlugin;
  if ( currentIndex().isValid() ) {
    currentPlugin = currentIndex().model()->data( currentIndex(), Model::PluginName ).toString();
  }

  QList<Kontact::Plugin*> pluginsToShow;
  foreach ( Kontact::Plugin *plugin, plugins_ ) {
    if ( plugin->showInSideBar() ) {
      pluginsToShow << plugin;
    }
  }

  mModel->setPluginList( pluginsToShow );

  mModel->removeRows( 0, mModel->rowCount() );
  mModel->insertRows( 0, pluginsToShow.count() );

  // Restore the previous selected index, if any
  if ( !currentPlugin.isEmpty() ) {
    setCurrentPlugin( currentPlugin );
  }
}

void Navigator::setCurrentPlugin( const QString &plugin )
{
  for ( int i = 0; i < model()->rowCount(); i++ ) {
    const QModelIndex index = model()->index( i, 0 );
    const QString pluginName = model()->data( index, Model::PluginName ).toString();

    if ( plugin == pluginName ) {
      selectionModel()->setCurrentIndex( index, QItemSelectionModel::SelectCurrent );
      break;
    }
  }
}

QSize Navigator::sizeHint() const
{
  //### TODO: We can cache this value, so this reply is faster. Since here we won't
  //          have too many elements, it is not that important. When caching this value
  //          make sure it is updated correctly when new rows have been added or
  //          removed. (ereslibre)

  int maxWidth = 0;
  for ( int i = 0; i < model()->rowCount(); i++ ) {
    const QModelIndex index = model()->index( i, 0 );
    maxWidth = qMax( maxWidth, sizeHintForIndex( index ).width() );
  }

  int viewHeight = QListView::sizeHint().height();

  return QSize( maxWidth + rect().width() - contentsRect().width(), viewHeight );
}

void Navigator::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->proposedAction() == Qt::IgnoreAction ) {
    return;
  }
  event->acceptProposedAction();
}

void Navigator::dragMoveEvent( QDragMoveEvent *event )
{
  if ( event->proposedAction() == Qt::IgnoreAction ) {
    return;
  }

  const QModelIndex dropIndex = indexAt( event->pos() );

  if ( !dropIndex.isValid() ||
       !( dropIndex.model()->flags( dropIndex ) & Qt::ItemIsEnabled ) ) {
    event->setAccepted( false );
    return;
  } else {
    const QModelIndex sourceIndex =
      static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( dropIndex );
    Kontact::Plugin *plugin =
      static_cast<Kontact::Plugin*>( sourceIndex.internalPointer() );
    if ( !plugin->canDecodeMimeData( event->mimeData() ) ) {
      event->setAccepted( false );
      return;
    }
  }

  event->acceptProposedAction();
}

void Navigator::dropEvent( QDropEvent *event )
{
  if ( event->proposedAction() == Qt::IgnoreAction ) {
    return;
  }

  const QModelIndex dropIndex = indexAt( event->pos() );

  if ( !dropIndex.isValid() ) {
    return;
  } else {
    const QModelIndex sourceIndex =
      static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( dropIndex );
    Kontact::Plugin *plugin =
      static_cast<Kontact::Plugin*>( sourceIndex.internalPointer() );
    plugin->processDropEvent( event );
  }
}

void Navigator::showEvent( QShowEvent *event )
{
  parentWidget()->setMaximumWidth( sizeHint().width() );
  parentWidget()->setMinimumWidth( sizeHint().width() );

  QListView::showEvent( event );
}

void Navigator::slotCurrentChanged( const QModelIndex &current )
{
  if ( !current.isValid() || !current.internalPointer() ||
       !( current.model()->flags( current ) & Qt::ItemIsEnabled ) ) {
    return;
  }

  QModelIndex source =
    static_cast<const QSortFilterProxyModel*>( current.model() )->mapToSource( current );

  emit pluginActivated( static_cast<Kontact::Plugin*>( source.internalPointer() ) );
}

void Navigator::slotActionTriggered( bool checked )
{
  QObject *object = sender();

  if ( object == mShowIconsAction ) {
    mShowIcons = checked;
    mShowText = !checked;
  } else if ( object == mShowTextAction ) {
    mShowIcons = !checked;
    mShowText = checked;
  } else if ( object == mShowBothAction ) {
    mShowIcons = checked;
    mShowText = checked;
  } else if ( object == mBigIconsAction ) {
    mIconSize = KIconLoader::SizeLarge;
  } else if ( object == mNormalIconsAction ) {
    mIconSize = KIconLoader::SizeMedium;
  } else if ( object == mSmallIconsAction ) {
    mIconSize = KIconLoader::SizeSmallMedium;
  }

  Prefs::self()->setSidePaneIconSize( mIconSize );
  Prefs::self()->setSidePaneShowIcons( mShowIcons );
  Prefs::self()->setSidePaneShowText( mShowText );

  mModel->emitLayoutChanged();

  parentWidget()->setMaximumWidth( sizeHint().width() );
  parentWidget()->setMinimumWidth( sizeHint().width() );
}

IconSidePane::IconSidePane( Core *core, QWidget *parent )
  : SidePaneBase( core, parent )
{
  mNavigator = new Navigator( this );
  mNavigator->setFocusPolicy( Qt::NoFocus );
  connect( mNavigator, SIGNAL(pluginActivated(Kontact::Plugin*)),
           SIGNAL(pluginSelected(Kontact::Plugin*)) );
}

IconSidePane::~IconSidePane()
{
}

void IconSidePane::setCurrentPlugin( const QString &plugin )
{
  mNavigator->setCurrentPlugin( plugin );
}

void IconSidePane::updatePlugins()
{
  mNavigator->updatePlugins( core()->pluginList() );
}

void IconSidePane::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event );
  setMaximumWidth( mNavigator->sizeHint().width() );
  setMinimumWidth( mNavigator->sizeHint().width() );
}

}

#include "iconsidepane.moc"

// vim: sw=2 sts=2 et tw=80
