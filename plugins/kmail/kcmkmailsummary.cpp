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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QCheckBox>
#include <QLayout>
#include <QVBoxLayout>

#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <k3listview.h>
#include <klocale.h>

#include "kcmkmailsummary.h"

#include <kdemacros.h>
#include "kmailinterface.h"

extern "C"
{
  KDE_EXPORT KCModule *create_kmailsummary( QWidget *parent, const char * )
  {
	KInstance *inst = new KInstance("kcmkmailsummary" );
    return new KCMKMailSummary( inst, parent );
  }
}

KCMKMailSummary::KCMKMailSummary( KInstance *inst, QWidget *parent )
  : KCModule( inst, parent )
{
  initGUI();

  connect( mFolderView, SIGNAL( clicked( Q3ListViewItem* ) ), SLOT( modified() ) );
  connect( mFullPath, SIGNAL( toggled( bool ) ), SLOT( modified() ) );

  KAcceleratorManager::manage( this );

  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkmailsummary" ),
                                      I18N_NOOP( "Mail Summary Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
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

  mFolderView = new K3ListView( this );
  mFolderView->setRootIsDecorated( true );
  mFolderView->setFullWidth( true );

  mFolderView->addColumn( i18n( "Summary" ) );

  mFullPath = new QCheckBox( i18n( "Show full path for folders" ), this );

  layout->addWidget( mFolderView );
  layout->addWidget( mFullPath );
}

void KCMKMailSummary::initFolders()
{
   org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
   QDBusReply<QStringList> lst = kmail.folderList();

   QStringList folderList(lst);
  mFolderView->clear();
  mFolderMap.clear();

  QStringList::Iterator it;
  for ( it = folderList.begin(); it != folderList.end(); ++it ) {
    QString displayName;
    if ( (*it) == "/Local" )
      displayName = i18nc( "prefix for local folders", "Local" );
    else {
#warning Port me to DBus!
//      DCOPRef folderRef = kmail.call( "getFolder(QString)", *it );
//      folderRef.call( "displayName()" ).get( displayName );
    }
    if ( (*it).count( '/' ) == 1 ) {
      if ( mFolderMap.find( *it ) == mFolderMap.end() )
        mFolderMap.insert( *it, new Q3ListViewItem( mFolderView,
                                                   displayName ) );
    } else {
      const int pos = (*it).lastIndexOf( '/' );
      const QString parentFolder = (*it).left( pos );
      mFolderMap.insert( *it,
                         new Q3CheckListItem( mFolderMap[ parentFolder ],
                                             displayName,
                                             Q3CheckListItem::CheckBox ) );
    }
  }
}

void KCMKMailSummary::loadFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList folders;
  if ( !config.hasKey( "ActiveFolders" ) )
    folders << "/Local/inbox";
  else
    folders = config.readEntry( "ActiveFolders" , QStringList() );

  QMap<QString, Q3ListViewItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it ) {
    if ( Q3CheckListItem *qli = dynamic_cast<Q3CheckListItem*>( it.value() ) ) {
      if ( folders.contains( it.key() ) ) {
        qli->setOn( true );
        mFolderView->ensureItemVisible( it.value() );
      } else {
        qli->setOn( false );
      }
    }
  }
  mFullPath->setChecked( config.readEntry( "ShowFullPath", true ) );
}

void KCMKMailSummary::storeFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList folders;

  QMap<QString, Q3ListViewItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it )
    if ( Q3CheckListItem *qli = dynamic_cast<Q3CheckListItem*>( it.value() ) )
      if ( qli->isOn() )
        folders.append( it.key() );

  config.writeEntry( "ActiveFolders", folders );
  config.writeEntry( "ShowFullPath", mFullPath->isChecked() );

  config.sync();
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
