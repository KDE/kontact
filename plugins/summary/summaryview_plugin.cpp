/* This file is part of the KDE project

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

#include "core.h"
#include "plugin.h"

#include "kmailinterface.h"

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kaboutdata.h>

#include <KAction>
#include <KActionCollection>
#include <KSelectAction>


#include <QtDBus/QtDBus>
#include <QList>
#include <QMenu>

typedef KGenericFactory< SummaryView, Kontact::Core > SummaryViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_summaryplugin,
                            SummaryViewFactory( "kontact_summaryplugin" ) )

SummaryView::SummaryView( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, 0 ),
    mAboutData( 0 ), mPart( 0 )
{
  setComponentData( SummaryViewFactory::componentData() );

  mSyncAction = new KSelectAction( KIcon("view-refresh"), i18n( "Synchronize All" ), this );
  actionCollection()->addAction( "kontact_summary_sync", mSyncAction );
  connect( mSyncAction, SIGNAL( triggered( QString )  ), SLOT( syncAccount( QString ) ) );
  connect( mSyncAction->menu(), SIGNAL( aboutToShow() ), this, SLOT( fillSyncActionSubEntries() ) );

  insertSyncAction( mSyncAction );
  fillSyncActionSubEntries();
}

void SummaryView::fillSyncActionSubEntries()
{
  QStringList menuItems;
  menuItems.append( i18n("All") );

  org::kde::kmail::kmail kmail( "org.kde.kmail", "/KMailIface", QDBusConnection::sessionBus() );
  const QDBusReply<QStringList> reply = kmail.accounts();
  if ( reply.isValid() ) {
    menuItems << reply.value();
  }

  mSyncAction->clear();
  mSyncAction->setItems( menuItems );
}

void SummaryView::syncAccount( const QString& account )
{
  QDBusMessage message = QDBusMessage::createSignal( "/KMailIface", "org.kde.kmail", "checkAccount(QString)");
  message << ( account == i18n("All") ? QString() : account );
  QDBusConnection::sessionBus().send( message );
  fillSyncActionSubEntries();
}

SummaryView::~SummaryView()
{
}

void SummaryView::doSync()
{
  if ( mPart )
    mPart->updateSummaries();

  const QList<Kontact::Plugin*> pluginList = core()->pluginList();
  Q_FOREACH( const Kontact::Plugin* i, pluginList ) {
    // execute all sync actions but our own
    const QList<KAction*>* actions = i->syncActions();
    Q_FOREACH( KAction* j, *actions )  {
      if ( j != mSyncAction )
        j->trigger();
    }
  }
  fillSyncActionSubEntries();
}

KParts::ReadOnlyPart *SummaryView::createPart()
{
  mPart = new SummaryViewPart( core(), "summarypartframe", aboutData(),
                               this );
  mPart->setObjectName( "summaryPart" );
  return mPart;
}

const KAboutData *SummaryView::aboutData()
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "kontactsummary", 0, ki18n("Kontact Summary"),
                                 "1.1",
                                 ki18n("Kontact Summary View"),
                                 KAboutData::License_LGPL,
                                 ki18n("(c) 2003 The Kontact developers" ) );
    mAboutData->addAuthor( ki18n("Sven Lueppken"), KLocalizedString(), "sven@kde.org" );
    mAboutData->addAuthor( ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org" );
    mAboutData->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );
    mAboutData->setProductName( "kontact/summary" );
  }

  return mAboutData;
}

#include "summaryview_plugin.moc"
