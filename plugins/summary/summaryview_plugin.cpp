/* This file is part of the KDE project
   Copyright (C) 2003 Sven Lüppken <sven@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kaboutdata.h>

#include "core.h"
#include "summaryview_part.h"

#include "summaryview_plugin.h"

typedef KGenericFactory< SummaryView, Kontact::Core > SummaryViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_summaryplugin,
                            SummaryViewFactory( "kontact_summaryplugin" ) )

SummaryView::SummaryView( Kontact::Core *core, const char *name, const QStringList& )
  : Kontact::Plugin( core, core, name),
    mAboutData( 0 )
{
  setInstance( SummaryViewFactory::instance() );
}

SummaryView::~SummaryView()
{
}

KParts::ReadOnlyPart *SummaryView::createPart()
{
  return new SummaryViewPart( core(), "summarypartframe", aboutData(),
                              this, "summarypart" );
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
