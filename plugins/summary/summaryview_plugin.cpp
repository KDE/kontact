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
#include "core.h"
#include "summaryview_part.h"

#include <dcopref.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kaboutdata.h>
#include <kaction.h>

#include <qpopupmenu.h>

typedef KGenericFactory< SummaryView, Kontact::Core > SummaryViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_summaryplugin,
                            SummaryViewFactory( "kontact_summaryplugin" ) )

SummaryView::SummaryView( Kontact::Core *core, const char *name, const QStringList& )
  : Kontact::Plugin( core, core, name),
    mAboutData( 0 ), mPart( 0 )
{
  setInstance( SummaryViewFactory::instance() );

  mSyncAction = new KSelectAction( i18n( "Synchronize All" ), "reload", 0, this,
                                   SLOT( doSync() ), actionCollection(),
                                   "kontact_summary_sync" );
  connect( mSyncAction, SIGNAL( activated( const QString& ) ), this, SLOT( syncAccount( const QString& ) ) );
  connect( mSyncAction->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( fillSyncActionSubEntries() ) );

  insertSyncAction( mSyncAction );
  fillSyncActionSubEntries();
}

void SummaryView::fillSyncActionSubEntries()
{
  QStringList menuItems;
  menuItems.append( i18n("All") );

  DCOPRef ref( "kmail", "KMailIface" );
  DCOPReply reply = ref.call( "accounts" );

  if ( reply.isValid() )
  {
    const QStringList accounts = reply;
    menuItems += accounts;
  }
  mSyncAction->clear();
  mSyncAction->setItems( menuItems );
}

void SummaryView::syncAccount( const QString& account )
{
  const QString acc = account == i18n("All") ? QString() : account;
  DCOPRef ref( "kmail", "KMailIface" );
  ref.send( "checkAccount", acc );
  fillSyncActionSubEntries();
}

SummaryView::~SummaryView()
{
}

void SummaryView::doSync()
{
  if ( mPart )
    mPart->updateSummaries();

  const QValueList<Kontact::Plugin*> pluginList = core()->pluginList();
  for ( QValueList<Kontact::Plugin*>::ConstIterator it = pluginList.begin(), end = pluginList.end();
        it != end; ++it ) {
    // execute all sync actions but our own
    QPtrList<KAction> *actions = (*it)->syncActions();
    for ( QPtrList<KAction>::Iterator jt = actions->begin(), end = actions->end(); jt != end; ++jt ) {
      if ( *jt != mSyncAction )
        (*jt)->activate();
    }
  }
  fillSyncActionSubEntries();
}

KParts::ReadOnlyPart *SummaryView::createPart()
{
  mPart = new SummaryViewPart( core(), "summarypartframe", aboutData(),
                               this, "summarypart" );
  return mPart;
}

const KAboutData *SummaryView::aboutData()
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "kontactsummary", I18N_NOOP("Kontact Summary"),
                                 "1.1",
                                 I18N_NOOP("Kontact Summary View"),
                                 KAboutData::License_LGPL,
                                 I18N_NOOP("(c) 2003 The Kontact developers" ) );
    mAboutData->addAuthor( "Sven Lueppken", "", "sven@kde.org" );
    mAboutData->addAuthor( "Cornelius Schumacher", "", "schumacher@kde.org" );
    mAboutData->addAuthor( "Tobias Koenig", "", "tokoe@kde.org" );
    mAboutData->setProductName( "kontact/summary" );
  }

  return mAboutData;
}

#include "summaryview_plugin.moc"
