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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qclipboard.h>
#include <qeventloop.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "NewsTickerPlugin" ), mLayout( 0 )
{
  QVBoxLayout *vlay = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_news",
                                                  KIcon::Desktop, KIcon::SizeMedium );

  QWidget *header = createHeader( this, icon, i18n( "News Feeds" ) );
  vlay->addWidget( header );

  QString error;
  QCString appID;

  bool dcopAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "rssservice" ) ) {
    if ( KApplication::startServiceByDesktopName( "rssservice", QStringList(), &error, &appID ) ) {
      QLabel *label = new QLabel( i18n( "No rss dcop service available.\nYou need rssservice to use this plugin." ), this );
      vlay->addWidget( label, Qt::AlignHCenter );
      dcopAvailable = false;
    }
  }

  mBaseWidget = new QWidget( this, "baseWidget" );
  vlay->addWidget( mBaseWidget );

  connect( &mTimer, SIGNAL( timeout() ), this, SLOT( updateDocuments() ) );

  readConfig();

  if ( dcopAvailable )
    initDocuments();

  connectDCOPSignal( 0, 0, "added(QString)", "documentAdded(QString)", false );
  connectDCOPSignal( 0, 0, "removed(QString)", "documentRemoved(QString)", false );
}

int SummaryWidget::summaryHight() const
{
  return ( mFeeds.count() == 0 ? 1 : mFeeds.count() );
}

void SummaryWidget::documentAdded( QString )
{
  initDocuments();
}

void SummaryWidget::documentRemoved( QString )
{
  initDocuments();
}

void SummaryWidget::configChanged()
{
  readConfig();

  updateView();
}

void SummaryWidget::readConfig()
{
  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  mUpdateInterval = config.readNumEntry( "UpdateInterval", 600 );
  mArticleCount = config.readNumEntry( "ArticleCount", 4 );
}

void SummaryWidget::initDocuments()
{
  mFeeds.clear();

  DCOPRef dcopCall( "rssservice", "RSSService" );
  QStringList urls;
  dcopCall.call( "list()" ).get( urls );

  if ( urls.isEmpty() ) { // add default
    urls.append( "http://www.kde.org/dotkdeorg.rdf" );
    dcopCall.send( "add(QString)", urls[ 0 ] );
  }

  QStringList::Iterator it;
  for ( it = urls.begin(); it != urls.end(); ++it ) {
    DCOPRef feedRef = dcopCall.call( "document(QString)", *it );

    Feed feed;
    feed.ref = feedRef;
    feedRef.call( "title()" ).get( feed.title );
    feedRef.call( "link()" ).get( feed.url );
    feedRef.call( "pixmap()" ).get( feed.logo );
    mFeeds.append( feed );

    connectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)",
                       "documentUpdated(DCOPRef)", false );

    qApp->processEvents( QEventLoop::ExcludeUserInput | 
                         QEventLoop::ExcludeSocketNotifiers );
  }

  updateDocuments();
}

void SummaryWidget::updateDocuments()
{
  mTimer.stop();

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
    (*it).ref.send( "refresh()" );

  mTimer.start( 1000 * mUpdateInterval );
}

void SummaryWidget::documentUpdated( DCOPRef feedRef )
{
  static uint feedCounter = 0;
  ArticleMap map;

  int numArticles = feedRef.call( "count()" );
  for ( int i = 0; i < numArticles; ++i ) {
    DCOPRef artRef = feedRef.call( "article(int)", i );
    QString title, url;

    qApp->processEvents( QEventLoop::ExcludeUserInput | 
                         QEventLoop::ExcludeSocketNotifiers );

    artRef.call( "title()" ).get( title );
    artRef.call( "link()" ).get( url );

    QPair<QString, KURL> article(title, KURL( url ));
    map.append( article );
  }

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
    if ( (*it).ref.obj() == feedRef.obj() ) {
      (*it).map = map;
      if ( (*it).title.isEmpty() )
        feedRef.call( "title()" ).get( (*it).title );
      if ( (*it).url.isEmpty() )
        feedRef.call( "link()" ).get( (*it).url );
      if ( (*it).logo.isNull() )
        feedRef.call( "pixmap()" ).get( (*it).logo );
    }

  feedCounter++;
  if ( feedCounter == mFeeds.count() ) {
    feedCounter = 0;
    updateView();
  }
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  delete mLayout;
  mLayout = new QVBoxLayout( mBaseWidget, 3 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    QHBox *hbox = new QHBox( mBaseWidget );
    mLayout->addWidget( hbox );

    // icon
    KURLLabel *urlLabel = new KURLLabel( hbox );
    urlLabel->setURL( (*it).url );
    urlLabel->setPixmap( (*it).logo );
    urlLabel->setMaximumSize( urlLabel->minimumSizeHint() );
    mLabels.append( urlLabel );

    connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
             kapp, SLOT( invokeBrowser( const QString& ) ) );
    connect( urlLabel, SIGNAL( rightClickedURL( const QString& ) ),
             this, SLOT( rmbMenu( const QString& ) ) );

    // header
    QLabel *label = new QLabel( hbox );
    label->setText( KCharsets::resolveEntities( (*it).title ) );
    label->setAlignment( AlignLeft|AlignVCenter );
    label->setFont( boldFont );
    label->setIndent( 6 );
    label->setMaximumSize( label->minimumSizeHint() );
    mLabels.append( label );

    hbox->setMaximumWidth( hbox->minimumSizeHint().width() );
    hbox->show();

    // articles
    ArticleMap articles = (*it).map;
    ArticleMap::Iterator artIt;
    int numArticles = 0;
    for ( artIt = articles.begin(); artIt != articles.end() && numArticles < mArticleCount; ++artIt ) {
      urlLabel = new KURLLabel( (*artIt).second.url(), (*artIt).first, mBaseWidget );
      urlLabel->setTextFormat( RichText );
      mLabels.append( urlLabel );
      mLayout->addWidget( urlLabel );

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               kapp, SLOT( invokeBrowser( const QString& ) ) );
      connect( urlLabel, SIGNAL( rightClickedURL( const QString& ) ),
               this, SLOT( rmbMenu( const QString& ) ) );


      numArticles++;
    }
  }

  for ( QLabel *label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

QStringList SummaryWidget::configModules() const
{
  return "kcmkontactknt.desktop";
}

void SummaryWidget::rmbMenu( const QString& url )
{
  QPopupMenu menu;
  menu.insertItem( i18n( "Copy URL to Clipboard" ) );
  int id = menu.exec( QCursor::pos() );
  if ( id != -1 )
    kapp->clipboard()->setText( url, QClipboard::Clipboard );
}

#include "summarywidget.moc"
