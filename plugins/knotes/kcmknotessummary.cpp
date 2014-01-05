/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "kcmknotessummary.h"

#include "akonadi_next/note.h"

#include "pimcommon/folderdialog/checkedcollectionwidget.h"

#include <Akonadi/ETMViewStateSaver>

#include <KAboutData>
#include <KAcceleratorManager>
#include <KCheckableProxyModel>
#include <KComponentData>
#include <KDebug>
#include <KDialog>
#include <KLocalizedString>
#include <KLineEdit>

#include <QCheckBox>
#include <QTreeView>
#include <QVBoxLayout>

extern "C"
{
KDE_EXPORT KCModule *create_knotessummary( QWidget *parent, const char * )
{
    KComponentData inst( "kcmknotessummary" );
    return new KCMKNotesSummary( inst, parent );
}
}

KCMKNotesSummary::KCMKNotesSummary( const KComponentData &inst, QWidget *parent )
    : KCModule( inst, parent )
{
    initGUI();

    connect( mCheckedCollectionWidget->folderTreeView(), SIGNAL(clicked(QModelIndex)),
             SLOT(modified()) );

    KAcceleratorManager::manage( this );

    load();

    KAboutData *about =
            new KAboutData( I18N_NOOP( "kcmknotessummary" ), 0,
                            ki18n( "Notes Summary Configuration Dialog" ),
                            0, KLocalizedString(), KAboutData::License_GPL,
                            ki18n( "Copyright © 2013 Laurent Montel <montel@kde.org>" ) );

    about->addAuthor( ki18n( "Laurent Montel" ),
                      KLocalizedString(), "montel@kde.org" );
    setAboutData( about );
}

void KCMKNotesSummary::modified()
{
    emit changed( true );
}

void KCMKNotesSummary::initGUI()
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );
    layout->setMargin( 0 );

    mCheckedCollectionWidget = new PimCommon::CheckedCollectionWidget(Akonotes::Note::mimeType());
    layout->addWidget( mCheckedCollectionWidget );
}

void KCMKNotesSummary::initFolders()
{
    KSharedConfigPtr _config = KSharedConfig::openConfig( QLatin1String("kcmknotessummaryrc") );

    mModelState =
            new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( _config->group( "CheckState" ), this );
    mModelState->setSelectionModel( mCheckedCollectionWidget->selectionModel() );
}

void KCMKNotesSummary::loadFolders()
{
    mModelState->restoreState();
}

void KCMKNotesSummary::storeFolders()
{
    KConfig config( QLatin1String("kcmknotessummaryrc") );
    mModelState->saveState();
    config.sync();
}

void KCMKNotesSummary::load()
{
    initFolders();
    loadFolders();

    emit changed( false );
}

void KCMKNotesSummary::save()
{
    storeFolders();

    emit changed( false );
}

void KCMKNotesSummary::defaults()
{
    emit changed( true );
}
