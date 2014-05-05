/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2014 Laurent Montel <montel@kde.org>

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
#include "knotesinterface.h"

#include "noteshared/akonadi/notesakonaditreemodel.h"
#include "noteshared/akonadi/noteschangerecorder.h"
#include "noteshared/attributes/showfoldernotesattribute.h"
#include "noteshared/attributes/notedisplayattribute.h"

#include <AkonadiCore/Item>
#include <AkonadiCore/Session>
#include <AkonadiCore/ChangeRecorder>
#include <AkonadiWidgets/ETMViewStateSaver>
#include <AkonadiCore/collectionstatistics.h>
#include <KCheckableProxyModel>

#include <KMime/KMimeMessage>

#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <KIconLoader>
#include <KLocalizedString>
#include <KUrlLabel>
#include <QMenu>
#include <KIconEffect>
#include <KSharedConfig>

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QItemSelectionModel>


KNotesSummaryWidget::KNotesSummaryWidget(KontactInterface::Plugin *plugin, QWidget *parent )
    : KontactInterface::Summary( parent ),
      mLayout( 0 ),
      mPlugin( plugin ),
      mInProgress( false )
{
    mDefaultPixmap = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
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
             SLOT(updateFolderList()));

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
}

KNotesSummaryWidget::~KNotesSummaryWidget()
{
}

void KNotesSummaryWidget::updateFolderList()
{
    if (mInProgress)
        return;
    mInProgress = true;
    qDeleteAll( mLabels );
    mLabels.clear();
    int counter = 0;

    mModelState->restoreState();
    displayNotes(QModelIndex(), counter);
    mInProgress = false;

    if ( counter == 0 ) {
        QLabel *label = new QLabel( i18n( "No note found" ), this );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        mLayout->addWidget( label, 0, 0 );
        mLabels.append( label );
    }
    QList<QLabel*>::const_iterator lit;
    QList<QLabel*>::const_iterator lend( mLabels.constEnd() );
    for ( lit = mLabels.constBegin(); lit != lend; ++lit ) {
        (*lit)->show();
    }
}

void KNotesSummaryWidget::displayNotes( const QModelIndex &parent, int &counter)
{
    const int nbCol = mModelProxy->rowCount( parent );
    for ( int i = 0; i < nbCol; ++i ) {
        const QModelIndex child = mModelProxy->index( i, 0, parent );
        const Akonadi::Item item =
                mModelProxy->data( child,
                                  Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if (item.isValid()) {
            createNote(item, counter);
            ++counter;
        }
        displayNotes( child, counter );
    }
}

void KNotesSummaryWidget::slotPopupMenu(const QString &note)
{
    QMenu popup( this );
    const QAction *modifyNoteAction = popup.addAction(
                KIconLoader::global()->loadIcon( QLatin1String("document-edit"), KIconLoader::Small ),
                i18n( "Modify Note..." ) );
    popup.addSeparator();
    const QAction *deleteNoteAction = popup.addAction(
                KIconLoader::global()->loadIcon( QLatin1String("edit-delete"), KIconLoader::Small ),
                i18n( "Delete Note..." ) );

    const QAction *ret = popup.exec( QCursor::pos() );
    if ( ret == deleteNoteAction ) {
        deleteNote( note );
    } else if ( ret == modifyNoteAction ) {
        slotSelectNote( note );
    }
}

void KNotesSummaryWidget::deleteNote(const QString &note)
{
    org::kde::kontact::KNotes knotes( QLatin1String("org.kde.kontact"), QLatin1String("/KNotes"), QDBusConnection::sessionBus() );
    knotes.killNote(note.toLongLong());
}

void KNotesSummaryWidget::createNote(const Akonadi::Item &item, int counter)
{
    if (!item.hasPayload<KMime::Message::Ptr>())
        return;

    KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
    if (!noteMessage)
        return;
    const KMime::Headers::Subject * const subject = noteMessage->subject(false);
    KUrlLabel *urlLabel = new KUrlLabel( QString::number( item.id() ), subject ? subject->asUnicodeString() : QString(), this );

    urlLabel->installEventFilter( this );
    urlLabel->setAlignment( Qt::AlignLeft );
    urlLabel->setWordWrap( true );
    connect( urlLabel, SIGNAL(leftClickedUrl(QString)), this, SLOT(slotSelectNote(QString)) );
    connect( urlLabel, SIGNAL(rightClickedUrl(QString)), this, SLOT(slotPopupMenu(QString)) );

    mLayout->addWidget( urlLabel, counter, 1 );

    QColor color;
    if ( item.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
        color = item.attribute<NoteShared::NoteDisplayAttribute>()->backgroundColor();
    }
    // Folder icon.
    KIconEffect effect;
    QPixmap pixmap = effect.apply( mDefaultPixmap, KIconEffect::Colorize, 1, color, false );

    QLabel *label = new QLabel( this );
    label->setAlignment( Qt::AlignVCenter );
    QIcon icon(pixmap);
    label->setPixmap( icon.pixmap( label->height() / 1.5 ) );
    label->setMaximumWidth( label->minimumSizeHint().width() );
    mLayout->addWidget( label, counter, 0 );
    mLabels.append( label );
    mLabels.append( urlLabel );
}

void KNotesSummaryWidget::updateSummary( bool force )
{
    Q_UNUSED( force );
    updateFolderList();
}

void KNotesSummaryWidget::slotSelectNote( const QString &note )
{
    if ( !mPlugin->isRunningStandalone() ) {
        mPlugin->core()->selectPlugin( mPlugin );
    } else {
        mPlugin->bringToForeground();
    }
    org::kde::kontact::KNotes knotes( QLatin1String("org.kde.kontact"), QLatin1String("/KNotes"), QDBusConnection::sessionBus() );
    knotes.editNote(note.toLongLong());
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
