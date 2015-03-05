/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <Akonadi/Notes/NoteUtils>

#include "pimcommon/folderdialog/checkedcollectionwidget.h"

#include <AkonadiWidgets/ETMViewStateSaver>

#include <KAboutData>
#include <KAcceleratorManager>
#include <KCheckableProxyModel>
#include <QDebug>
#include <QDialog>
#include <KLocalizedString>
#include <QLineEdit>
#include <KSharedConfig>

#include <QCheckBox>
#include <QTreeView>
#include <QVBoxLayout>
#include <KConfigGroup>

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knotessummary(QWidget *parent, const char *)
    {
        return new KCMKNotesSummary(parent);
    }
}

KCMKNotesSummary::KCMKNotesSummary(QWidget *parent)
    : KCModule(parent)
{
    initGUI();

    connect(mCheckedCollectionWidget->folderTreeView(), SIGNAL(clicked(QModelIndex)),
            SLOT(modified()));

    KAcceleratorManager::manage(this);

    load();

    KAboutData *about = new KAboutData(QStringLiteral("kcmknotessummary"),
                                       i18n("kcmknotessummary"),
                                       QString(),
                                       i18n("Notes Summary Configuration Dialog"),
                                       KAboutLicense::GPL,
                                       i18n("Copyright © 2013-2015 Laurent Montel <montel@kde.org>"));
    about->addAuthor(ki18n("Laurent Montel").toString(), QString(), QStringLiteral("montel@kde.org"));
    setAboutData(about);
}

void KCMKNotesSummary::modified()
{
    emit changed(true);
}

void KCMKNotesSummary::initGUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    mCheckedCollectionWidget = new PimCommon::CheckedCollectionWidget(Akonadi::NoteUtils::noteMimeType());
    layout->addWidget(mCheckedCollectionWidget);
}

void KCMKNotesSummary::initFolders()
{
    KSharedConfigPtr _config = KSharedConfig::openConfig(QStringLiteral("kcmknotessummaryrc"));

    mModelState =
        new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(_config->group("CheckState"), this);
    mModelState->setSelectionModel(mCheckedCollectionWidget->selectionModel());
}

void KCMKNotesSummary::loadFolders()
{
    mModelState->restoreState();
}

void KCMKNotesSummary::storeFolders()
{
    KConfig config(QStringLiteral("kcmknotessummaryrc"));
    mModelState->saveState();
    config.sync();
}

void KCMKNotesSummary::load()
{
    initFolders();
    loadFolders();

    emit changed(false);
}

void KCMKNotesSummary::save()
{
    storeFolders();

    emit changed(false);
}

void KCMKNotesSummary::defaults()
{
    emit changed(true);
}
