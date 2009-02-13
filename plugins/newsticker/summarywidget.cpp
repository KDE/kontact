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

#include <QClipboard>
#include <QCursor>
#include <QEventLoop>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPixmap>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <ktoolinvocation.h>

SummaryWidget::SummaryWidget( QWidget *parent )
  : Kontact::Summary( parent ),
    /*DCOPObject( "NewsTickerPlugin" ),*/ mLayout( 0 )
{
  QVBoxLayout *vlay = new QVBoxLayout( this );
  vlay->setSpacing( 3 );
  vlay->setMargin( 3 );

  QWidget *header = createHeader( this, "view-pim-news", i18n( "News Feeds" ) );
  vlay->addWidget( header );

  QString error;
#ifdef __GNUC__
#warning Port me!
#endif
#if 0
  DCOPCString appID;

  bool dcopAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "rssservice" ) ) {
    if ( KToolInvocation::startServiceByDesktopName(
           "rssservice", QStringList(), &error, &appID ) ) {
      QLabel *label = new QLabel(
        i18n( "No rss dcop service available.\nYou need rssservice to use this plugin." ), this );
      vlay->addWidget( label, Qt::AlignHCenter );
      dcopAvailable = false;
    }
  }
#endif

  mBaseWidget = new QWidget( this );
  mBaseWidget->setObjectName( "baseWidget" );
  vlay->addWidget( mBaseWidget );

  connect( &mTimer, SIGNAL(timeout()), this, SLOT(updateDocuments()) );

  readConfig();

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
  connectDCOPSignal(
    0, 0, "documentUpdateError(DCOPRef,int)", "documentUpdateError(DCOPRef, int)", false );

  if ( dcopAvailable ) {
    initDocuments();
  }

  connectDCOPSignal( 0, 0, "added(QString)", "documentAdded(const QString &)", false );
  connectDCOPSignal( 0, 0, "removed(QString)", "documentRemoved(const QString &)", false );
#endif
}

int SummaryWidget::summaryHeight() const
{
  return ( mFeeds.count() == 0 ? 1 : mFeeds.count() );
}

void SummaryWidget::documentAdded( const QString & )
{
  initDocuments();
}

void SummaryWidget::documentRemoved( const QString & )
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
  KConfigGroup grp(&config, "General" );

  mUpdateInterval = grp.readEntry( "UpdateInterval", 600 );
  mArticleCount = grp.readEntry( "ArticleCount", 4 );
}

void SummaryWidget::initDocuments()
{
  mFeeds.clear();

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
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

    disconnectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)", 0 );
    connectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)",
                       "documentUpdated(DCOPRef)", false );

    if ( qApp ) {
      qApp->processEvents( QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers );
    }
  }
#endif

  updateDocuments();
}

void SummaryWidget::updateDocuments()
{
  mTimer.stop();

  FeedList::Iterator it;
#ifdef __GNUC__
#warning Port me!
#endif
#if 0
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    (*it).ref.send( "refresh()" );
  }
#endif

  mTimer.start( 1000 * mUpdateInterval );
}

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
void SummaryWidget::documentUpdated( DCOPRef feedRef )
{
  ArticleMap map;

  int numArticles = feedRef.call( "count()" );
  for ( int i = 0; i < numArticles; ++i ) {
    DCOPRef artRef = feedRef.call( "article(int)", i );
    QString title, url;

    if ( qApp ) {
      qApp->processEvents( QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers );
    }

    artRef.call( "title()" ).get( title );
    artRef.call( "link()" ).get( url );

    QPair<QString, KUrl> article( title, KUrl( url ) );
    map.append( article );
  }

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    if ( (*it).ref.obj() == feedRef.obj() ) {
      (*it).map = map;
      if ( (*it).title.isEmpty() ) {
        feedRef.call( "title()" ).get( (*it).title );
      }
      if ( (*it).url.isEmpty() ) {
        feedRef.call( "link()" ).get( (*it).url );
      }
      if ( (*it).logo.isNull() ) {
        feedRef.call( "pixmap()" ).get( (*it).logo );
      }
    }
  }

  mFeedCounter++;
  if ( mFeedCounter == mFeeds.count() ) {
    mFeedCounter = 0;
    updateView();
  }
}
#endif

void SummaryWidget::updateView()
{
  qDeleteAll( mLabels );
  mLabels.clear();

  delete mLayout;
  mLayout = new QVBoxLayout( mBaseWidget );
  mLayout->setSpacing( 3 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    KHBox *hbox = new KHBox( mBaseWidget );
    mLayout->addWidget( hbox );

    // icon
    KUrlLabel *urlLabel = new KUrlLabel( hbox );
    urlLabel->setUrl( (*it).url );
    urlLabel->setPixmap( (*it).logo );
    urlLabel->setWordWrap( true );
    urlLabel->setMaximumSize( urlLabel->minimumSizeHint() );
    mLabels.append( urlLabel );

    connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
             kapp, SLOT(invokeBrowser(const QString&)) );
    connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
             this, SLOT(rmbMenu(const QString&)) );

    // header
    QLabel *label = new QLabel( hbox );
    label->setText( KCharsets::resolveEntities( (*it).title ) );
    label->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
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
    for ( artIt = articles.begin();
          artIt != articles.end() && numArticles < mArticleCount; ++artIt ) {
      urlLabel = new KUrlLabel( (*artIt).second.url(), (*artIt).first, mBaseWidget );
      urlLabel->installEventFilter( this );
      //TODO: RichText causes too much horizontal space between articles
      //urlLabel->setTextFormat( Qt::RichText );
      mLabels.append( urlLabel );
      mLayout->addWidget( urlLabel );

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               kapp, SLOT(invokeBrowser(const QString&)) );
      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(rmbMenu(const QString&)) );

      numArticles++;
    }
  }

  Q_FOREACH( QLabel *label, mLabels ) {
    label->show();
  }
}

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
void SummaryWidget::documentUpdateError( DCOPRef feedRef, int errorCode )
{
  kDebug() <<" error while updating document, error code:" << errorCode;
  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    if ( (*it).ref.obj() == feedRef.obj() ) {
      mFeeds.erase( it );
      break;
    }
  }

  if ( mFeedCounter == mFeeds.count() ) {
    mFeedCounter = 0;
    updateView();
  }

}
#endif

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkontactknt.desktop" );
}

void SummaryWidget::updateSummary( bool )
{
  updateDocuments();
}

void SummaryWidget::rmbMenu( const QString &url )
{
  QMenu menu;
  menu.addMenu( i18n( "Copy URL to Clipboard" ) );
  if ( menu.exec( QCursor::pos() ) ) {
    QApplication::clipboard()->setText( url, QClipboard::Clipboard );
  }
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( label->text() );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

#include "summarywidget.moc"
