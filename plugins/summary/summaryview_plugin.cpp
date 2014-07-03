/*
  This file is part of the KDE project

  Copyright (C) 2003 Sven L�ppken <sven@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include "summaryview_plugin.h"
#include "summaryview_part.h"
#include "kdepim-version.h"
#ifdef KMAIL_SUPPORTED
#include "kmailinterface.h"
#endif

#include <KontactInterface/Core>

#include <KAboutData>
#include <KActionCollection>
#include <QIcon>
#include <KLocalizedString>
#include <KSelectAction>

#include <QMenu>

EXPORT_KONTACT_PLUGIN( SummaryView, summary )

SummaryView::SummaryView( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, 0 ), mAboutData( 0 ), mPart( 0 )
{
  //QT5 setComponentData( KontactPluginFactory::componentData() );

  mSyncAction = new KSelectAction( QIcon::fromTheme( QLatin1String("view-refresh") ), i18n( "Sync All" ), this );
  actionCollection()->addAction( QLatin1String("kontact_summary_sync"), mSyncAction );
  connect( mSyncAction, SIGNAL(triggered(QString)), SLOT(syncAccount(QString)) );
  connect( mSyncAction->menu(), SIGNAL(aboutToShow()), this, SLOT(fillSyncActionSubEntries()) );

  insertSyncAction( mSyncAction );
  fillSyncActionSubEntries();
}

void SummaryView::fillSyncActionSubEntries()
{
  QStringList menuItems;
  menuItems.append( i18nc( "@action:inmenu sync everything", "All" ) );

#ifdef KMAIL_SUPPORTED
  org::kde::kmail::kmail kmail( QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QDBusConnection::sessionBus() );
  const QDBusReply<QStringList> reply = kmail.accounts();
  if ( reply.isValid() ) {
    menuItems << reply.value();
  }
#endif

  mSyncAction->clear();
  mSyncAction->setItems( menuItems );
}

void SummaryView::syncAccount( const QString &account )
{
  if ( account == i18nc( "sync everything", "All" ) ) {
    doSync();
  } else {
#ifdef KMAIL_SUPPORTED
    org::kde::kmail::kmail kmail( QLatin1String("org.kde.kmail"), QLatin1String("/KMail"),
                                  QDBusConnection::sessionBus() );
    kmail.checkAccount( account );
#endif
  }
  fillSyncActionSubEntries();
}

SummaryView::~SummaryView()
{
}

void SummaryView::doSync()
{
  if ( mPart ) {
    mPart->updateSummaries();
  }

  const QList<KontactInterface::Plugin *> pluginList = core()->pluginList();
  Q_FOREACH ( const KontactInterface::Plugin *i, pluginList ) {
    // execute all sync actions but our own
    Q_FOREACH ( QAction *j, i->syncActions() ) {
#if 0 //QT5
      if ( j != mSyncAction ) {
        j->trigger();
      }
#endif
    }
  }
  fillSyncActionSubEntries();
}

KParts::ReadOnlyPart *SummaryView::createPart()
{
  //QT5 mPart = new SummaryViewPart( core(), "summarypartframe", aboutData(), this );
  mPart->setObjectName( QLatin1String("summaryPart") );
  return mPart;
}

const KAboutData *SummaryView::aboutData() const
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData(
      QLatin1String("kontactsummary"), 
      i18n( "Kontact Summary" ),
      QLatin1String(KDEPIM_VERSION),
      i18n( "Kontact Summary View" ),
      KAboutLicense::LGPL,
      i18n( "(c) 2003-2014 The Kontact developers" ) );

    mAboutData->addAuthor( i18n( "Sven Lueppken" ),
                           QString(), QLatin1String("sven@kde.org") );
    mAboutData->addAuthor( i18n( "Cornelius Schumacher" ),
                           QString(), QLatin1String("schumacher@kde.org") );
    mAboutData->addAuthor( i18n( "Tobias Koenig" ),
                           QString(), QLatin1String("tokoe@kde.org") );
    mAboutData->setProductName( "kontact/summary" );
  }

  return mAboutData;
}

#include "summaryview_plugin.moc"
