/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "summarywidget.h"
#include "knotes_plugin.h"

#include "noteshared/akonadi/notesakonaditreemodel.h"
#include "noteshared/akonadi/noteschangerecorder.h"

#include <Akonadi/Session>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/CollectionStatistics>
#include <KCheckableProxyModel>


#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <KIconLoader>
#include <KLocalizedString>
#include <KUrlLabel>

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QItemSelectionModel>


KNotesSummaryWidget::KNotesSummaryWidget(KontactInterface::Plugin *plugin, QWidget *parent )
    : KontactInterface::Summary( parent ),
      mLayout( 0 ),
      mPlugin( plugin )
{
    QVBoxLayout *mainLayout = new QVBoxLayout( this );
    mainLayout->setSpacing( 3 );
    mainLayout->setMargin( 3 );

    QWidget *header = createHeader( this, QLatin1String("view-pim-notes"), i18n( "Popup Notes" ) );
    mainLayout->addWidget( header );

    mLayout = new QGridLayout();
    mainLayout->addItem( mLayout );
    mLayout->setSpacing( 3 );
    mLayout->setRowStretch( 6, 1 );

    KIconLoader loader( QLatin1String("knotes") );

    mPixmap = loader.loadIcon( QLatin1String("knotes"), KIconLoader::Small );

    Akonadi::Session *session = new Akonadi::Session( "KNotes Session", this );
    mNoteRecorder = new NoteShared::NotesChangeRecorder(this);
    mNoteRecorder->changeRecorder()->setSession(session);
    mNoteTreeModel = new NoteShared::NotesAkonadiTreeModel(mNoteRecorder->changeRecorder(), this);

    connect( mNoteTreeModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
             SLOT(slotRowInserted(QModelIndex,int,int)));

    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), SLOT(updateFolderList()));
    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemRemoved(Akonadi::Item)), SLOT(updateFolderList()) );

    mSelectionModel = new QItemSelectionModel( mNoteTreeModel );
    mModelProxy = new KCheckableProxyModel( this );
    mModelProxy->setSelectionModel( mSelectionModel );
    mModelProxy->setSourceModel( mNoteTreeModel );

    KSharedConfigPtr _config = KSharedConfig::openConfig( QLatin1String("kcmknotessummaryrc") );

    mModelState =
            new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( _config->group( "CheckState" ), this );
    mModelState->setSelectionModel( mSelectionModel );

    connect( mNoteRecorder, SIGNAL(collectionChanged(Akonadi::Collection)),
             SLOT(slotCollectionChanged(Akonadi::Collection)) );
    connect( mNoteRecorder, SIGNAL(collectionRemoved(Akonadi::Collection)),
             SLOT(slotCollectionChanged(Akonadi::Collection)) );

    connect( mNoteTreeModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
             SLOT(slotRowInserted(QModelIndex,int,int)));
    updateFolderList();
}

KNotesSummaryWidget::~KNotesSummaryWidget()
{
}

void KNotesSummaryWidget::updateFolderList()
{
    qDeleteAll( mLabels );
    int counter = 0;
    mLabels.clear();
    mModelState->restoreState();
    displayNotes(QModelIndex(), counter);
}

void KNotesSummaryWidget::displayNotes( const QModelIndex &parent, int &counter)
{
    const int nbCol = mModelProxy->rowCount( parent );
    for ( int i = 0; i < nbCol; ++i ) {
        const QModelIndex child = mModelProxy->index( i, 0, parent );
        const Akonadi::Collection col =
                mModelProxy->data( child,
                                   Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
        const int showCollection =
                mModelProxy->data( child, Qt::CheckStateRole ).value<int>();

        if ( col.isValid() ) {
            const Akonadi::CollectionStatistics stats = col.statistics();
            if ( ( ( stats.unreadCount() ) != Q_INT64_C(0) ) && showCollection ) {
                //TODO
            }
        }
        displayNotes( child, counter );
    }
}

void KNotesSummaryWidget::slotCollectionChanged( const Akonadi::Collection &col )
{
    Q_UNUSED( col );
    updateFolderList();
}

void KNotesSummaryWidget::slotRowInserted( const QModelIndex & parent, int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    updateFolderList();
}

void KNotesSummaryWidget::updateSummary( bool force )
{
    Q_UNUSED( force );
    updateFolderList();
}

void KNotesSummaryWidget::urlClicked( const QString &/*uid*/ )
{
    if ( !mPlugin->isRunningStandalone() ) {
        mPlugin->core()->selectPlugin( mPlugin );
    } else {
        mPlugin->bringToForeground();
    }
}

bool KNotesSummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
    if ( obj->inherits( "KUrlLabel" ) ) {
        KUrlLabel* label = static_cast<KUrlLabel*>( obj );
        if ( e->type() == QEvent::Enter ) {
            emit message( i18n( "Read Popup Note: \"%1\"", label->text() ) );
        } else if ( e->type() == QEvent::Leave ) {
            emit message( QString::null );        //krazy:exclude=nullstrassign for old broken gcc
        }
    }

    return KontactInterface::Summary::eventFilter( obj, e );
}


QStringList KNotesSummaryWidget::configModules() const
{
    return QStringList()<<QLatin1String( "kcmknotessummary.desktop" );
}

#include "moc_summarywidget.cpp"
