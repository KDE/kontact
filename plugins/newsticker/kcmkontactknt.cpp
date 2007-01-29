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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <q3groupbox.h>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QVector>
#include <QSpinBox>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>

#include <QtDBus>

#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <k3listview.h>
#include <klocale.h>
#include <kpushbutton.h>

#include "kcmkontactknt.h"

#include "newsfeeds.h"

#include <kdemacros.h>
#include <ktoolinvocation.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactknt( QWidget *parent, const char * )
  {
	KComponentData inst("kcmkontactknt" );
    return new KCMKontactKNT( inst,parent );
  }
}

NewsEditDialog::NewsEditDialog( const QString& title, const QString& url, QWidget *parent )
  : KDialog( parent)
{
  setCaption(i18n( "New News Feed" ));
  setButtons(Ok | Cancel);
  setDefaultButton(Ok);
  setModal(true);
  showButtonSeparator(true);

  QWidget *page = new QWidget(this);
  setMainWidget(page);
  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( marginHint() );

  QLabel *label = new QLabel( i18n( "Name:" ), page );
  layout->addWidget( label, 0, 0 );

  mTitle = new QLineEdit( page );
  label->setBuddy( mTitle );
  layout->addWidget( mTitle, 0, 1, 1, 2 );

  label = new QLabel( i18n( "URL:" ), page );
  layout->addWidget( label, 1, 0 );

  mURL = new QLineEdit( page );
  label->setBuddy( mURL );
  layout->addWidget( mURL, 1, 1, 1, 2 );

  mTitle->setText( title );
  mURL->setText( url );
  mTitle->setFocus();
  connect( mTitle, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mURL, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );

  modified();
}

void NewsEditDialog::modified()
{
  enableButton( KDialog::Ok, !title().isEmpty() && !url().isEmpty() );
}

QString NewsEditDialog::title() const
{
  return mTitle->text();
}

QString NewsEditDialog::url() const
{
  return mURL->text();
}

class NewsItem : public Q3ListViewItem
{
  public:
    NewsItem( Q3ListView *parent, const QString& title, const QString& url, bool custom )
      : Q3ListViewItem( parent ), mTitle( title ), mUrl( url ), mCustom( custom )
    {
      setText( 0, mTitle );
    }

    NewsItem( Q3ListViewItem *parent, const QString& title, const QString& url, bool custom )
      : Q3ListViewItem( parent ), mTitle( title ), mUrl( url ), mCustom( custom )
    {
      setText( 0, mTitle );
    }

    QString title() const { return mTitle; }
    QString url() const { return mUrl; }
    bool custom() const { return mCustom; }

  private:
    QString mTitle;
    QString mUrl;
    bool mCustom;
};

KCMKontactKNT::KCMKontactKNT( const KComponentData &inst,QWidget *parent )
  : KCModule( inst, parent )
{
  initGUI();

  connect( mAllNews, SIGNAL( currentChanged( Q3ListViewItem* ) ),
           this, SLOT( allCurrentChanged( Q3ListViewItem* ) ) );
  connect( mSelectedNews, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
           this, SLOT( selectedChanged( Q3ListViewItem* ) ) );

  connect( mUpdateInterval, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );
  connect( mArticleCount, SIGNAL( valueChanged( int ) ), SLOT( modified() ) );

  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addNews() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( removeNews() ) );
  connect( mNewButton, SIGNAL( clicked() ), this, SLOT( newFeed() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), this, SLOT( deleteFeed() ) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMKontactKNT::loadNews()
{
  QVector<Q3ListViewItem*> parents;
  QVector<Q3ListViewItem*>::Iterator it;

  parents.append( new Q3ListViewItem( mAllNews, i18n( "Arts" ) ) );
  parents.append( new Q3ListViewItem( mAllNews, i18n( "Business" ) ) );
  parents.append( new Q3ListViewItem( mAllNews, i18n( "Computers" ) ) );
  parents.append( new Q3ListViewItem( mAllNews, i18n( "Misc" ) ) );
  parents.append( new Q3ListViewItem( mAllNews, i18n( "Recreation" ) ) );
  parents.append( new Q3ListViewItem( mAllNews, i18n( "Society" ) ) );

  for ( it = parents.begin(); it != parents.end(); ++it )
    (*it)->setSelectable( false );

  for ( int i = 0; i < DEFAULT_NEWSSOURCES; ++i ) {
    NewsSourceData data = NewsSourceDefault[ i ];
    new NewsItem( parents[ data.category() ], data.name(), data.url(), false );
    mFeedMap.insert( data.url(), data.name() );
  }
}

void KCMKontactKNT::loadCustomNews()
{
  KConfig config( "kcmkontactkntrc" );
  QMap<QString, QString> customFeeds = config.entryMap( "CustomFeeds" );
  config.setGroup( "CustomFeeds" );

  mCustomItem = new Q3ListViewItem( mAllNews, i18n( "Custom" ) );
  mCustomItem->setSelectable( false );

  if ( customFeeds.count() == 0 )
    mCustomItem->setVisible( false );

  QMap<QString, QString>::Iterator it;
  for ( it = customFeeds.begin(); it != customFeeds.end(); ++it ) {
    QStringList value = config.readEntry( it.key(), QStringList() );
    mCustomFeeds.append( new NewsItem( mCustomItem, value[ 0 ], value[ 1 ], true ) );
    mFeedMap.insert( value[ 1 ], value[ 0 ] );
    mCustomItem->setVisible( true );
  }
}

void KCMKontactKNT::storeCustomNews()
{
  KConfig config( "kcmkontactkntrc" );
  config.deleteGroup( "CustomFeeds" );
  config.setGroup( "CustomFeeds" );

  int counter = 0;
  QList<NewsItem*>::Iterator it;
  for ( it = mCustomFeeds.begin(); it != mCustomFeeds.end(); ++it ) {
    QStringList value;
    value << (*it)->title() << (*it)->url();
    config.writeEntry( QString::number( counter ), value );

    ++counter;
  }

  config.sync();
}

void KCMKontactKNT::addNews()
{
  if ( !dbusActive() )
    return;

  NewsItem *item = dynamic_cast<NewsItem*>( mAllNews->selectedItem() );
  if ( item == 0 )
    return;

#ifdef __GNUC__
  #warning "insert the right dbus path/interface here, once knewsticker has been ported"
#endif
  QDBusInterface service( "org.kde.rssservice", "/", "org.kde.rssservice.RSSService" );
  service.call( "add(QString)", item->url() );

  scanNews();

  emit changed( true );
}

void KCMKontactKNT::removeNews()
{
  if ( !dbusActive() )
    return;

  NewsItem *item = dynamic_cast<NewsItem*>( mSelectedNews->selectedItem() );
  if ( item == 0 )
    return;

#ifdef __GNUC__
  #warning "insert the right dbus path/interface here, once knewsticker has been ported"
#endif
  QDBusInterface service( "org.kde.rssservice", "/", "org.kde.rssservice.RSSService" );
  service.call( "remove(QString)", item->url() );

  scanNews();

  emit changed( true );
}

void KCMKontactKNT::newFeed()
{
  NewsEditDialog dlg( "", "", this );

  if ( dlg.exec() ) {
    NewsItem *item = new NewsItem( mCustomItem, dlg.title(), dlg.url(), true );
    mCustomFeeds.append( item );
    mFeedMap.insert( dlg.url(), dlg.title() );

    mCustomItem->setVisible( true );
    mCustomItem->setOpen( true );
    mAllNews->ensureItemVisible( item );
    mAllNews->setSelected( item, true );

    emit changed( true );
  }
}

void KCMKontactKNT::deleteFeed()
{
  NewsItem *item = dynamic_cast<NewsItem*>( mAllNews->selectedItem() );
  if ( !item )
    return;

  if ( !mCustomFeeds.contains( item )  )
    return;

  mCustomFeeds.removeAll( item );
  mFeedMap.remove( item->url() );
  delete item;

  if ( mCustomFeeds.count() == 0 )
    mCustomItem->setVisible( false );

  emit changed( true );
}

void KCMKontactKNT::scanNews()
{
  if ( !dbusActive() )
    return;

  mSelectedNews->clear();

#ifdef __GNUC__
  #warning "insert the right dbus path/interface here, once knewsticker has been ported"
#endif
  QDBusInterface service( "org.kde.rssservice", "/", "org.kde.rssservice.RSSService" );
  QDBusReply<QStringList> reply = service.call( "list()" );
  QStringList urls = reply.value();

  for ( int i = 0; i < urls.count(); ++i )
  {
    QString url = urls[ i ];
    QString feedName = mFeedMap[ url ];
    if ( feedName.isEmpty() )
      feedName = url;
    new NewsItem( mSelectedNews, feedName, url, false );
  }
}

void KCMKontactKNT::selectedChanged( Q3ListViewItem *item )
{
  mRemoveButton->setEnabled( item && item->isSelected() );
}

void KCMKontactKNT::allCurrentChanged( Q3ListViewItem *item )
{
  NewsItem *newsItem = dynamic_cast<NewsItem*>( item );

  bool addState = false;
  bool delState = false;
  if ( newsItem && newsItem->isSelected() ) {
    addState = true;
    delState = (mCustomFeeds.contains( newsItem ) );
  }

  mAddButton->setEnabled( addState );
  mDeleteButton->setEnabled( delState );
}

void KCMKontactKNT::modified()
{
  emit changed( true );
}

void KCMKontactKNT::initGUI()
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  mAllNews = new K3ListView( this );
  mAllNews->addColumn( i18n( "All" ) );
  mAllNews->setRootIsDecorated( true );
  mAllNews->setFullWidth( true );
  layout->addWidget( mAllNews, 0, 0 );

  QVBoxLayout *vbox = new QVBoxLayout();
  layout->addLayout( vbox, 0, 1 );
  vbox->setSpacing( KDialog::spacingHint() );

  vbox->addStretch();
  mAddButton = new KPushButton( i18n( "Add" ), this );
  mAddButton->setEnabled( false );
  vbox->addWidget( mAddButton );
  mRemoveButton = new KPushButton( i18n( "Remove" ), this );
  mRemoveButton->setEnabled( false );
  vbox->addWidget( mRemoveButton );
  vbox->addStretch();

  mSelectedNews = new K3ListView( this );
  mSelectedNews->addColumn( i18n( "Selected" ) );
  mSelectedNews->setFullWidth( true );
  layout->addWidget( mSelectedNews, 0, 2 );

  Q3GroupBox *box = new Q3GroupBox( 0, Qt::Vertical,
                                  i18n( "News Feed Settings" ), this );

  QGridLayout *boxLayout = new QGridLayout();
  box->layout()->addItem( boxLayout );
  boxLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Refresh time:" ), box );
  boxLayout->addWidget( label, 0, 0 );

  mUpdateInterval = new QSpinBox( box );
  mUpdateInterval->setRange( 1, 3600 );
  mUpdateInterval->setSuffix( " sec." );
  label->setBuddy( mUpdateInterval );
  boxLayout->addWidget( mUpdateInterval, 0, 1 );

  label = new QLabel( i18n( "Number of items shown:" ), box );
  boxLayout->addWidget( label, 1, 0 );

  mArticleCount = new QSpinBox( box );
  label->setBuddy( mArticleCount );
  boxLayout->addWidget( mArticleCount, 1, 1 );

  mNewButton = new KPushButton( i18n( "New Feed..." ), box );
  boxLayout->addWidget( mNewButton, 0, 2 );

  mDeleteButton = new KPushButton( i18n( "Delete Feed" ), box );
  mDeleteButton->setEnabled( false );
  boxLayout->addWidget( mDeleteButton, 1, 2 );

  layout->addWidget( box, 1, 0, 1, 3 );
}

bool KCMKontactKNT::dbusActive() const
{
  QString error;
  QString appID;
  bool isGood = true;
  
#ifdef __GNUC__
  #warning "insert the right dbus path/interface here, once knewsticker has been ported"
#endif
  QDBusInterface service( "org.kde.rssservice", "/", "org.kde.rssservice.RSSService" );
  if ( !service.isValid() ) {
    if ( KToolInvocation::startServiceByDesktopName( "rssservice", QStringList(), &error, &appID ) )
      isGood = false;
  }

  return isGood;
}

void KCMKontactKNT::load()
{
  mAllNews->clear();

  loadNews();
  loadCustomNews();
  scanNews();

  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  mUpdateInterval->setValue( config.readEntry( "UpdateInterval", 600 ) );
  mArticleCount->setValue( config.readEntry( "ArticleCount", 4 ) );

  emit changed( false );
}

void KCMKontactKNT::save()
{
  storeCustomNews();

  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  config.writeEntry( "UpdateInterval", mUpdateInterval->value() );
  config.writeEntry( "ArticleCount", mArticleCount->value() );

  config.sync();

  emit changed( false );
}

void KCMKontactKNT::defaults()
{
}

const KAboutData* KCMKontactKNT::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkontactknt" ),
                                      I18N_NOOP( "Newsticker Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkontactknt.moc"
