/*
  This file is part of Kontact.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcmkmailsummary.h"
#include "akonadi_next/checkableitemproxymodel.h"

#include <KAboutData>
#include <KAcceleratorManager>
#include <KComponentData>
#include <KDebug>
#include <KDialog>
#include <KLocale>

#include <Akonadi/EntityTreeModel>
#include <Akonadi/ChangeRecorder>

#include <QCheckBox>
#include <QTreeView>
#include <QVBoxLayout>

extern "C"
{
  KDE_EXPORT KCModule *create_kmailsummary( QWidget *parent, const char * )
  {
    KComponentData inst( "kcmkmailsummary" );
    return new KCMKMailSummary( inst, parent );
  }
}

KCMKMailSummary::KCMKMailSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  initGUI();

  connect( mFolderView, SIGNAL(itemClicked(QTreeWidgetItem *,int)),
           SLOT(modified()) );
  connect( mFullPath, SIGNAL(toggled(bool)), SLOT(modified()) );

  KAcceleratorManager::manage( this );

  load();

  KAboutData *about =
    new KAboutData( I18N_NOOP( "kcmkmailsummary" ), 0,
                    ki18n( "Mail Summary Configuration Dialog" ),
                    0, KLocalizedString(), KAboutData::License_GPL,
                    ki18n( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ),
                    KLocalizedString(), "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKMailSummary::modified()
{
  emit changed( true );
}

void KCMKMailSummary::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  mFolderView = new QTreeView( this );

  mFullPath = new QCheckBox( i18n( "Show full path for folders" ), this );
  mFullPath->setToolTip(
    i18nc( "@info:tooltip", "Show full path for each folder" ) );
  mFullPath->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Enable this option if you want to see the full path "
           "for each folder listed in the summary. If this option is "
           "not enabled, then only the base folder path will be shown." ) );
  layout->addWidget( mFolderView );
  layout->addWidget( mFullPath );
}

void KCMKMailSummary::initFolders()
{
  // Create a new change recorder.
  mChangeRecorder = new Akonadi::ChangeRecorder( this );
  mChangeRecorder->setMimeTypeMonitored( "Message/rfc822" );

  mModel = new Akonadi::EntityTreeModel( mChangeRecorder, this );
  
  // Set the model to show only collections, not items.
  mModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::NoItemPopulation );

  // Create the Check proxy model.
  mSelectionModel = new QItemSelectionModel( mModel );
  mCheckProxy = new CheckableItemProxyModel( this );
  mCheckProxy->setSelectionModel( mSelectionModel );
  mCheckProxy->setSourceModel( mModel );

  mFolderView->setModel( mCheckProxy );
}

void KCMKMailSummary::loadFolders()
{
#if 0 // TODO: Port to Akonadi
  KConfig _config( "kcmkmailsummaryrc" );
  KConfigGroup config(&_config, "General" );

  QStringList folders;
  if ( !config.hasKey( "ActiveFolders" ) ) {
    folders << "/Local/inbox";
  } else {
    folders = config.readEntry( "ActiveFolders", QStringList() );
  }

  QMap<QString, QTreeWidgetItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it ) {
    if ( it.value()->flags() & Qt::ItemIsUserCheckable ) {
      if ( folders.contains( it.key() ) ) {
        it.value()->setCheckState( 0, Qt::Checked );
        mFolderView->scrollToItem( it.value() );
      } else {
        it.value()->setCheckState( 0, Qt::Unchecked );
      }
    }
  }
  mFullPath->setChecked( config.readEntry( "ShowFullPath", true ) );
#else
  kWarning() << "Port to Akonadi";
#endif
}

void KCMKMailSummary::storeFolders()
{
#if 0 // TODO: Port to Akonadi
  KConfig _config( "kcmkmailsummaryrc" );
  KConfigGroup config(&_config, "General" );

  QStringList folders;

  QMap<QString, QTreeWidgetItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it ) {
    if ( it.value()->flags() & Qt::ItemIsUserCheckable ) {
      if ( it.value()->checkState( 0 ) == Qt::Checked ) {
        folders.append( it.key() );
      }
    }
  }

  config.writeEntry( "ActiveFolders", folders );
  config.writeEntry( "ShowFullPath", mFullPath->isChecked() );

  config.sync();
#else
  kWarning() << "Port to Akonadi";
#endif
}

void KCMKMailSummary::load()
{
  initFolders();
  loadFolders();

  emit changed( false );
}

void KCMKMailSummary::save()
{
  storeFolders();

  emit changed( false );
}

void KCMKMailSummary::defaults()
{
  mFullPath->setChecked( true );

  emit changed( true );
}

#include "kcmkmailsummary.moc"
